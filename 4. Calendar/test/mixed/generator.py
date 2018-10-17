# coding=utf-8

from datetime import datetime, timedelta
import json
import numpy as np
import pymongo
import random
import string
import sys


# mongo DB info
MONGO_CLIENT_HOST = "mongodb://localhost:27017/"
MONGO_DB_NAME = "testdb"
MONGO_COLLECTION = "testcollection"


# random events dates range
MIN_DATE = datetime(2000, 1, 1)
MAX_DATE = datetime(2020, 1, 1)
TOTAL_DAYS = (MAX_DATE - MIN_DATE).days


# random event description length
MIN_DESCRIPTION_LENGTH = 10
MAX_DESCRIPTION_LENGTH = 1000
DESCRIPTION_ALPHABET = string.ascii_uppercase + string.digits + ' '


# managing probabilities of requests types
PROBA_POST_OUT_OF_100 = 20
PROBA_GET_ONE_OUT_OF_ALL_GETS = 80


# requests headers
POST_HEADERS = """Content-Type: application/json\r\nHost: localhost:8081"""
GET_HEADERS = """Host: localhost:8081"""


def get_all_uids():
    """
    Extract all known saved events uids
    (using pymongo module)
    """
    myclient = pymongo.MongoClient(MONGO_CLIENT_HOST)
    mydb = myclient[MONGO_DB_NAME]
    mycol = mydb[MONGO_COLLECTION]

    return [x['_id'] for x in mycol.find()]


def make_ammo(method, url, headers, case, body):
    """
    Create one bullet of specified type.
    Got that routine from yandex-tank documentation.
    """

    #http request w/o entity body template
    req_template = (
          "%s %s HTTP/1.1\r\n"
          "%s\r\n"
          "\r\n"
    )

    #http request with entity body template
    req_template_w_entity_body = (
          "%s %s HTTP/1.1\r\n"
          "%s\r\n"
          "Content-Length: %d\r\n"
          "\r\n"
          "%s\r\n"
    )

    if not body:
        req = req_template % (method, url, headers)
    else:
        req = req_template_w_entity_body % (method, url, headers, len(body), body)

    #phantom ammo template
    ammo_template = (
        "%d %s\n"
        "%s"
    )

    return ammo_template % (len(req), case, req)


def get_event_random_dates():
    """
    Each event duration is strictly 1 day.
    Generating random threshold between desired date and MIN_DATE
    """
    rand_date_shift = random.randint(1, TOTAL_DAYS)
    start = MIN_DATE + timedelta(days=rand_date_shift)
    return start, start + timedelta(days=1)


def get_event_random_description():
    """
    Generate random description text.
    (could have left same description for each event, maybe we will see smth interesting)
    """
    rand_length = random.randint(MIN_DESCRIPTION_LENGTH, MAX_DESCRIPTION_LENGTH)
    return ''.join(random.choice(DESCRIPTION_ALPHABET) for _ in xrange(rand_length))


def create_event_kwargs():
    """
    Create random event
    (Random description text, random date start. Length of the event is 1 day)
    """
    description = get_event_random_description()
    start, end = get_event_random_dates()
    return {
        'description': description,
        'start': start.strftime('%Y-%m-%dT%H:%M:%SZ'),
        'end': start.strftime('%Y-%m-%dT%H:%M:%SZ')
    }


def generate(count):
    """
    20% change to get POST
    Out of 75% left:
     - 80% get single object GET
     - 20% to get one year ranged events list
    """
    all_uids = get_all_uids()
    random.shuffle(all_uids)

    count_post = 0
    count_get_single = 0
    count_get_list = 0

    all_requests = []

    for _ in range(count):
        if random.randint(0, 100) <= PROBA_POST_OUT_OF_100:
            all_requests.append(make_ammo(
                'POST',
                '/events/',
                POST_HEADERS,
                'post_request',
                json.dumps(create_event_kwargs())
            ))
            count_post += 1
        else:
            if random.randint(0, 100) <= PROBA_GET_ONE_OUT_OF_ALL_GETS:
                rand_event_index = random.randint(0, len(all_uids) - 1)
                all_requests.append(make_ammo(
                    'GET',
                    '/event/{}/'.format(all_uids[rand_event_index]),
                    GET_HEADERS,
                    'get_single',
                    ''
                ))
                count_get_single += 1
            else:
                random_year = random.randint(MIN_DATE.year, MAX_DATE.year)
                random_month = random.randint(1, 12)
                random_day = random.randint(1, 28)

                filter_from = datetime(random_year, random_month, random_day).strftime('%Y-%m-%dT%H:%M:%SZ')

                all_requests.append(make_ammo(
                    'GET',
                    '/events/?date_from={}'.format(filter_from),
                    GET_HEADERS,
                    'get_range',
                    ''
                ))
                count_get_list += 1

    sys.stderr.write(u'Total: {}. POST: {}. GET_1: {}. GET_ALL: {}\n'.format(
        len(all_requests),
        count_post,
        count_get_single,
        count_get_list,
    ))

    for request in all_requests:
        sys.stdout.write(request)


if __name__ == "__main__":
    generate(100000)


## old
# https://overload.yandex.net/129460 (line(100, 5000, 10m), ограничение 100 на выгрузку списка)
# https://overload.yandex.net/129513 (same, нет ограничения)
# 123 (line(100, 1500, 2m) const(1500, 10m))


## Mixed
# https://overload.yandex.net/129560 line(100, 5000, 5m)
# https://overload.yandex.net/129564 line(100, 1000, 3m) const(1000, 6m)
# https://overload.yandex.net/129568 line(100, 800, 3m) const(800, 6m)
# https://overload.yandex.net/129582 line(100, 900, 3m) const(900, 6m)
# https://overload.yandex.net/129584 line(100, 1000, 3m) const(1000, 6m)
# https://overload.yandex.net/129599 line(100, 1200, 3m) const(1200, 7m)

## Get
# https://overload.yandex.net/129601 line(100, 7000, 5m)
# https://overload.yandex.net/129607 line(100, 3800, 3m) const(3800, 5m)

