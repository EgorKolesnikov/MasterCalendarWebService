#!/usr/bin/python
# -*- coding: utf-8 -*-

import json
import random
import sys


# #http request w/o entity body template
# req_template = (
#       "%s %s HTTP/1.1\n"
#       "%s\n"
#       "\n"
# )

# #http request with entity body template
# req_template_w_entity_body = (
#       "%s %s HTTP/1.1\n"
#       "%s\n"
#       "Content-Length: %d\n"
#       "\n"
#       "%s\n"
# )

#http request w/o entity body template
req_template = (
      "%s %s HTTP/1.1\n"
      "%s\n"
      "\n"
)

#http request with entity body template
req_template_w_entity_body = (
      "%s %s HTTP/1.1\n"
      "%s\n"
      "Content-Length: %d\n"
      "\n"
      "%s\n"
)


def make_ammo(method, url, headers, case, body):
    """ makes phantom ammo """
    if not body:
        req = req_template % (method, url, headers)
    else:
        req = req_template_w_entity_body % (method, url, headers, len(body), body)

    #phantom ammo template
    ammo_template = (
        "%d %s\n"
        "%s\n"
    )

    # return ammo_template % (len(req), case, req)
    return ammo_template % (len(body), case, body)


def generate(lines_count):
    body = json.dumps({
        "description": "123",
        "start": "2018-01-01T12:00:00Z",
        "end": "2018-01-01T13:00:00Z",
    })

    for _ in range(lines_count):
        headers = """Content-Type: application/json"""
        sys.stdout.write(make_ammo('POST', '/events/', headers, '/events/', body))


if __name__ == "__main__":
    generate(10)
