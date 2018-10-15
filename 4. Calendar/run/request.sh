# curl -o /dev/null -s -w "%{http_code}\n"-XPOST localhost:8081/events/ -H "Content-type: application/json" -d '{"description": "qweqwe", "start1": "123", "end": "123"}'
# curl -XPOST localhost:8081/events/ -H "Content-type: application/json" -d '{"description": "qweqwe", "start": "2018-01-01T12:00:00Z", "end": "2018-01-01T13:00:00Z"}'
# curl -XGET localhost:8081/event/5bc3a024e8ec40577e256f35/ -H "Content-type: application/json"
# curl -XGET localhost:8081/events/ -H "Content-type: application/json"
# curl -XPOST localhost:8081/event/5bc30b11e8ec400a4677ecd2/ -H "Content-type: application/json" -d '{"description": "newdescr", "start": "2018-01-01T11:00:00Z"}'
# curl -XGET "localhost:8081/events/?date_from=2018-01-01T00:00:00Z&date_to=2018-02-01T00:00:00Z" -H "Content-type: application/json"