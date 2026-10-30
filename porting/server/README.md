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

## Native dedicated server milestone

`FortniteLinuxServer.Target.cs` builds a Linux UE dedicated server from the
new `FortniteGame` module:

```sh
/opt/fortnite/UnrealEngine/Engine/Build/BatchFiles/Linux/Build.sh \
  FortniteLinuxServer Linux Development \
  -project=/opt/fortnite/FortniteClient-12.60-CL-13477524/porting/native/FortniteLinux.uproject \
  -NoHotReload
```

The built binary can be copied from `porting/native/Binaries/Linux/` into the
ignored staged build's `FortniteLinux/Binaries/Linux/` directory. With the
recovered `FortniteEmptyDedicated` map already staged, the server entered play
and listened on UDP 7777. A local instance of the new Linux client completed
UE challenge, login, and join, then loaded that map. Run
`bash porting/server/smoke_join.sh` to repeat the bounded probe, or pass a
staged map path as its first argument. It writes logs only under ignored
`porting/generated/`.

The same probe with
`/Game/Maps/Skybox/Skybox_Outpost_HomeBase` also succeeded: the server entered
the recovered skybox map and logged `Join succeeded`; the client received its
welcome and brought the skybox world up for play. The probe used `-nullrhi`
for both processes and ran for a bounded ten seconds.

This uses UE's `GameModeBase` and the newly authored client module. The
original Fortnite client protocol, account/profile services, game rules,
replication classes, and gameplay state are still unimplemented. The probe
does not establish compatibility with the supplied Windows executable.
