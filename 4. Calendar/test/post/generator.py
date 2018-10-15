# coding=utf-8

import json
import random
import sys
from datetime import datetime, timedelta


def make_ammo(method, url, headers, case, body):
    """ makes phantom ammo """
    # uripost ammo template
    ammo_template = (
        "%d %s\n"
        "%s\n"
    )
    return ammo_template % (len(body), url, body)


def generate(count):
    headers = """Content-Type: application/json"""
    description = "same description for all"

    for i in range(count):
        body = json.dumps({
            "description": description,
            "start": (datetime.now() + timedelta(days=i)).strftime('%Y-%m-%dT%H:%M:%SZ'),
            "end": (datetime.now() + timedelta(days=i + 10)).strftime('%Y-%m-%dT%H:%M:%SZ'),
        })
        sys.stdout.write(make_ammo('POST', '/events/', headers, 'post_requests', body))


if __name__ == "__main__":
    generate(1000)
