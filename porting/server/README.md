# Local server protocol probe

`probe_server.py` is a local HTTP recorder and fixture responder. It provides
the first server-side tool for reconstructing the client protocol. It does not
implement login, profiles, matchmaking, XMPP, or a UE game session yet.

Run `python3 porting/server/probe_server.py --port 8080`. It binds to
`127.0.0.1` by default and records method, path, query parameter names, body
length and hash, and response status to ignored
`porting/generated/server-requests.jsonl`. It does not store authorization
headers or request bodies. `/__health` returns a readiness response; unknown
requests return HTTP 501.

The optional `--fixtures path.json` accepts a JSON array. A fixture can match
a path segment with `{name}` and substitute `${name}` in its response:

```json
[
  {
    "method": "GET",
    "path": "/example/{name}",
    "status": 200,
    "body": { "name": "${name}" }
  }
]
```

Fixtures should be based on observed client requests and responses. This
tool's HTTP route coverage does not replace the authoritative game server.
