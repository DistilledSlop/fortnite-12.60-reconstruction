# Original-client matchmaking probe

On 2026-11-09 the original client was run in the private network namespace with
local login, TLS and AppES diagnostic runtime patches, and a native dedicated
server listening on the empty test map. Pressing Play returned a matchmaking
ticket, but the client's secure WebSocket failed to connect to the local
self-signed service. The exact cause of that connection failure is not proven.

A second run used the runner's loopback-only plaintext WebSocket listener; the
client connected, processed the queue and ready-check messages, and received a
session assignment. Its subsequent session lookup returned a response, but the
client reported `Unable to read session settings!` and a missing
`openPrivatePlayers` field, then `FindSessionFailure`. It did not join the
server. The dedicated server and backend shared a temporary test key, but the
client did not reach the encryption handshake. This is a diagnostic test, not a
verified match or a playable Battle Royale server.
