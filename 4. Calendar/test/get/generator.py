# coding=utf-8

import json
import random
import sys
import pymongo


def get_all_uids():
    """
    Extract all known uids
    """
    myclient = pymongo.MongoClient("mongodb://localhost:27017/")
    mydb = myclient["testdb"]
    mycol = mydb["testcollection"]

    return [x['_id'] for x in mycol.find()]


def generate():
    all_requests = []
    all_uids = get_all_uids()

    for uid in all_uids:
        all_requests.append("/event/{}/\n".format(uid))

    all_requests.extend(["/events/\n" for _ in xrange(len(all_uids) / 5)])
    random.shuffle(all_requests)

    for url in all_requests:
        sys.stdout.write(url)


if __name__ == "__main__":
    generate()
