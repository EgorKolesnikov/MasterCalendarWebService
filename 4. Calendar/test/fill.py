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


def fill(count, drop_before_fill=False):
    """
    Generate ${count} events and sotre them in MONGO_COLLECTION collection.
    :param count: how many events to generate
    :param drop_before_fill: True/False. If True - clear mongo collection before fill
    """
    client = pymongo.MongoClient(MONGO_CLIENT_HOST)
    db = client[MONGO_DB_NAME]
    collection = db[MONGO_COLLECTION]

    if drop_before_fill:
        print 'Dropping previous data'
        collection.drop()

    print 'Generating {} events'.format(count)
    all_events = []

    for i in xrange(count):
        if i % 1000 == 0:
            print i, count
        all_events.append(create_event_kwargs())

    print 'Saving {} generated events'.format(count)
    result = collection.insert_many(all_events)

    print 'Done. Inserted {} new events'.format(len(result.inserted_ids))


if __name__ == "__main__":
    fill(int(sys.argv[1]), drop_before_fill=True)
