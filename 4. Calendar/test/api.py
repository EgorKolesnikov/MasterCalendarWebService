# coding=utf-8

import json
import requests
import pymongo
import traceback
import re
from datetime import datetime, timedelta


VERBOSE = True

HOST = "http://localhost:8081"

DBNAME = "testdb"
DBCOLLECTION = "testcollection"

EXPECTED_FIELDS = [
    'description',
    'start',
    'end'
]


#
#   Utils
#

def _cleanup_testdb():
    """ Remove all documents in test collection """
    print ' ? Cleaning up DB documents'
    client = pymongo.MongoClient("mongodb://localhost:27017/")
    collection = client[DBNAME][DBCOLLECTION]
    collection.drop()


def _get_request(url):
    """ Simple GET request """
    return requests.get(url)


def _post_request(url, event_dict):
    """ Simple POST request """
    return requests.post(url, json=event_dict)


def _filter_requests(date_from=None, date_to=None):
    """
    Get request to filter events
    date_from and date_to may be specified - filter for event 'start' field
    """
    params = dict()
    if date_from is not None:
        params['date_from'] = date_from
    if date_to is not None:
        params['date_to'] = date_to

    url = HOST + '/events/'
    if params:
        url = '{}?{}'.format(url, '&'.join('{}={}'.format(key, value) for key, value in params.iteritems()))

    return requests.get(url)


def _check_event_json(result_json):
    """
    Check format of returned event json.
    Should contain:
     - "event" key with dict, describing event fields
     - "links" key with hypermedia (only "self" in format "/event/<id>/")
    """
    # check all registered event fields
    assert 'event' in result_json
    assert all(field in result_json['event'] for field in EXPECTED_FIELDS)

    # check hypermedia
    assert 'links' in result_json
    assert 'self' in result_json['links']
    assert re.match(r"^/event/[a-z0-9]{24}/$", result_json['links']['self']) is not None


def _generate_events(count, start_datetime):
    """
    Create 'count' number of events
    Each event has unique 'start' date (step = 1 day, starting from given datetime)
    """
    result = []
    description = 'all the same'

    while count > 0:
        result.append({
            "description": description,
            "start": (start_datetime + timedelta(days=count)).strftime('%Y-%m-%dT%H:%M:%SZ'),
            "end": (start_datetime + timedelta(days=count + 1)).strftime('%Y-%m-%dT%H:%M:%SZ')
        })
        count -= 1

    return result


def _create_event_with_checks(event_dict):
    """
    Check correct event creation
    (returned result code should be 201 and event json description should be in desired format (see _check_event_json(...)))
    """
    # create event
    response = _post_request(HOST + '/events/', event_dict)
    event_json = json.loads(response.text)

    # checks
    assert response.status_code == 201, 'Expected 201 (Created) code. Got: {}'.format(response.status_code)
    _check_event_json(event_json)

    return event_json


#
#   Tests
#

def test_create_ok():
    """
    Simple correct event creation
    """
    event_dict = {
        "description": "123",
        "start": "2018-01-01T00:00:00Z",
        "end": "2018-01-02T00:00:00Z",
    }

    return _create_event_with_checks(event_dict)


def test_create_bad_request():
    """
    Bad request if:
     - bad date format
     - not all fields specified
     - extra fields specified (which are not in EXPECTED_FIELDS)
    """
    
    # not all fields
    event_dict = {"description": "123", "start": "2018-01-01T00:00:00Z"}
    response = _post_request(HOST + '/events/', event_dict)
    assert response.status_code == 400, 'Expected to have 400 (BAD REQUEST. NOT ALL FIELDS), got {}'.format(response.status_code)

    # bad date format
    event_dict = {"description": "123", "start": "2018/01/01T00:00:00Z", "end": "2018-01-01T00:00:00"}
    response = _post_request(HOST + '/events/', event_dict)
    assert response.status_code == 400, 'Expected to have 400 (BAD REQUEST. BAD DATE FORMAT), got {}'.format(response.status_code)

    # extra fields
    event_dict = {"description": "123", "start": "2018-01-01T00:00:00Z", "end": "2018-01-01T00:00:00Z", "extra": "123"}
    response = _post_request(HOST + '/events/', event_dict)
    assert response.status_code == 400, 'Expected to have 400 (BAD REQUEST. EXTRA FIELD), got {}'.format(response.status_code)


def test_get_ok():
    """
    Check GET for one object. Creating one before testing.
    After load check returned event values to be the same as in the result of event create.
    """
    # create new event
    created_event = test_create_ok()
    uri = created_event['links']['self']

    # load same event
    response = _get_request(HOST + uri)
    loaded_event = json.loads(response.text)

    _check_event_json(loaded_event)

    # check fields values to be the same
    for field in EXPECTED_FIELDS:
        created = created_event['event'][field]
        loaded = loaded_event['event'][field]
        assert created == loaded, u'Field "{}" does not match ("{}", "{}")'.format(field, created, loaded)


def test_get_not_found():
    """
    Should return "NOT FOUND" when specified unknown event uid
    """
    # random string
    response = _get_request(HOST + "/event/123/")
    assert response.status_code == 404, 'Expected 404 (NOT FOUND), got "{}"'.format(response.status_code)

    # same regex, but random value
    response = _get_request(HOST + "/event/1234567890qwertyuiopasdq/")
    assert response.status_code == 404, 'Expected 404 (NOT FOUND), got "{}"'.format(response.status_code)


def test_modify_ok():
    """
    Check fields values right after modification.
    """
    # create new event
    created_event = test_create_ok()
    uri = created_event['links']['self']

    # modify event
    new_dict = {"description": "new_descr", "end": "2019-11-12T09:30:00Z"}
    response = _post_request(HOST + uri, new_dict)

    # load modified event
    response = _get_request(HOST + uri)
    loaded_event = json.loads(response.text)

    _check_event_json(loaded_event)

    # check fields
    for field in new_dict.keys():
        loaded = loaded_event['event'][field]
        new = new_dict[field]
        assert new == loaded, u'Field "{}" does not match ("{}", "{}")'.format(field, new, loaded)



def test_modify_not_found():
    """
    Trying to modify non existing object.
    (same uris as in test_get_not_found(...))
    """
    new_dict = {"description": "new_descr", "end": "2019-11-12T09:30:00Z"}

    response = _post_request(HOST + "/event/123/", new_dict)
    assert response.status_code == 404, 'Expected 404 (NOT FOUND), got "{}"'.format(response.status_code)

    response = _post_request(HOST + "/event/1234567890qwertyuiopasdq/", new_dict)
    assert response.status_code == 404, 'Expected 404 (NOT FOUND), got "{}"'.format(response.status_code)


def test_get_list_full():
    """
    Things to do:
     - remove all collection documents
     - use _generate_events() to generate new events
     - post all of them
     - load all of them
     - compare format of each loaded event
     - compare number of loaded events and created events
    """
    _cleanup_testdb()

    # post new events
    events_dicts = _generate_events(100, datetime.now())
    for event_dict in events_dicts:
        _ = _create_event_with_checks(event_dict)

    # loading list of all events
    response = _get_request(HOST + "/events/")
    assert response.status_code == 200, 'Expected 200 (OK) code. Got: {}'.format(response.status_code)
    
    # cehck loaded events count
    events = json.loads(response.text)
    created = len(events_dicts)
    loaded = len(events['events'])
    assert loaded == created, 'Number of created and loaded events does not match: {} and {}'.format(created, loaded)


def test_get_list_filter(create_count=100, filter_count=50):
    """
     - Cleanup test db collections
     - Create $(create_count) new events
     - Get start date of the ($(filter_count) - 1)-th event in sorted list
     - Query for all events less than that date
     - Should get exactly $(filter_count) events
    """
    _cleanup_testdb()

    # post new events
    events_dicts = _generate_events(create_count, datetime.now())
    for event_dict in events_dicts:
        _ = _create_event_with_checks(event_dict)

    # get date of 50-th element
    events_dicts = sorted(events_dicts, key=lambda x: x['start'])
    min_date = events_dicts[0]['start']
    target_date = events_dicts[filter_count - 1]['start']

    # loading list of all events
    response = _filter_requests(date_from=min_date, date_to=target_date)
    assert response.status_code == 200, 'Expected 200 (OK) code. Got: {}'.format(response.status_code)
    loaded_events = json.loads(response.text)
    count = len(loaded_events['events'])

    # check count
    assert count == filter_count, 'Expecting to have {} events. Got {}'.format(filter_count, count)


#
#   Run all tests
#

TESTS = [
    test_create_ok,
    test_create_bad_request,
    test_get_ok,
    test_get_not_found,
    test_modify_ok,
    test_modify_not_found,
    test_get_list_full,
    test_get_list_filter,
]

def run_test(t):
    try:
        print '\n * Running test "{}"'.format(t.__name__)
        _t()
        print ' + OK!'
    except Exception as e:
        print '   - EXCEPTION!', e.message
        print traceback.format_exc()


if __name__ == "__main__":    
    _cleanup_testdb()
    for _t in TESTS:
        run_test(_t)
    print '\nDONE\n'
    _cleanup_testdb()
