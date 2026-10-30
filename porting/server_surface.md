# Client service surface observed so far

This is an inventory of the 12.60 client, not a claim that a local server is
implemented or that every endpoint is known.

## Sources

- The cooked `DefaultEngine.ini` from `pakChunkEarly-WindowsClient.pak` is
  extracted to `porting/generated/DefaultEngine.ini` (ignored by Git).
- `inspect_client_routes.py` found 59 literal API path templates in the Win64
  client: 19 under `/api/game/v2/`, 34 under `/api/public/`, and 6 under
  `/api/oauth/`. Runtime-generated routes are not represented.

## Named subsystems

The config identifies MCP account, identity, party, profile, matchmaking,
session, XMPP, and game-service settings. It includes a `GameServiceMcp
Localhost` profile. The client binary also contains matchmaking ticket and
session API templates. These are leads for protocol reconstruction; the config
does not define response schemas or the game-session simulation.

## Server implementation order

1. Document the exact local startup sequence and each network request made by
   a runnable client. Static strings alone cannot establish ordering.
2. Implement local identity and profile responses needed to reach the lobby.
3. Implement party, matchmaking ticket, and session discovery responses needed
   to join a locally hosted session.
4. Implement the actual UE game-session protocol and authoritative gameplay
   state. HTTP route emulation alone will not put a player into a game.

The new native client project should use local service endpoints and omit EAC
integration. The supplied Windows client has not been modified.
