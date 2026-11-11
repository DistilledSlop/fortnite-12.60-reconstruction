# Wire findings: handshake and packet format (work in progress, 2026-11-11)

All addresses are in `generated/dump/FortniteClient-Win64-Shipping.runtime.exe`. Label `binary` unless stated.
Captures are in `/opt/fortnite/local-model-package/captures/*/<addr>.c`.

## 1. StatelessConnectHandlerComponent in the client

Vtable (first slot = destructor) at `0x146ad5610`. The client binary contains BOTH the client and the server half.

| Function | Address | Notes |
|---|---|---|
| `Incoming` | `0x1443a5dd0` | client and per-connection server path |
| `Outgoing` | `0x1443a89b0` | magic header, handshake bit 0, payload. Same as stock |
| `IncomingConnectionless` | `0x1443a61f0` | **server half, present in the client** |
| `Initialize` | `0x1443a7100` | |
| `NotifyHandshakeBegin` | `0x1443a7960` | sends initial / restart packet |
| `Tick` | `0x1443b3670` | resend timers |
| `GetReservedPacketBits` | `0x1443a5020` | `MagicHeaderBits + 1`. Same as stock |
| `ParseHandshakePacket` | `0x1443a8e30` | see layout below |
| `SendChallengeResponse` | `0x1443ad3f0` | |
| `SendChallengeAck` | `0x1443ad190` | server half |
| `SendConnectChallenge` | `0x1443ad6e0` | server half |
| `SendRestartHandshakeRequest` | `0x1443adbc0` | server half |
| `GenerateCookie` | `0x1443a12b0` | server half |

### Packet layout (from `ParseHandshakePacket`)

After the magic header (4 bits, `1110`) and the handshake bit:

| Packet | Bits left after handshake bit | Fields |
|---|---|---|
| Handshake (initial, challenge, response, ack) | 194 (`0xc2`) | `bRestartHandshake` 1, `SecretId` 1, `Timestamp` **32-bit float**, `Cookie` 20 bytes, then 1 termination bit |
| Restart response | 354 (`0x162`) | same, plus `OrigCookie` 20 bytes |
| Restart request (server to client) | 1 | `bRestartHandshake` 1 (accepted only in client mode) |

So `HANDSHAKE_PACKET_SIZE_BITS = 195` and `RESTART_RESPONSE_SIZE_BITS = 355`. **The sizes in `patches/ue425-fortnite-float-handshake.patch` are correct.**

### Logic comparison with stock 4.25 (source `StatelessConnectHandlerComponent.cpp`)

Identical in every branch I compared, except the time type:

- Timestamp on the wire is float. The driver clock used is a **float at `UNetDriver+0x210`** (the 4.24-style `Time`), not the 4.25 double `ElapsedTime`.
- Initial packet: `Timestamp == 0.0` → server sends challenge.
- Challenge response validity: `0 <= DriverTime - Timestamp < 40.0`; secret check `SecretId == ActiveSecret ? Timestamp - LastSecretUpdate >= 0 : <= 0`. Same as stock.
- Cookie is regenerated with `GenerateCookie(Address, SecretId, Timestamp)` and compared (20 bytes).
- On success, not restart: `LastServerSequence = *(uint16*)Cookie & 0x3fff`, `LastClientSequence = *(uint16*)(Cookie+2) & 0x3fff`, cookie stored. Restart: `OrigCookie` stored.
- Ack: client accepts a handshake packet with `Timestamp < 0`, calls `ServerConn->InitSequence(Cookie[0..1] & 0x3fff, Cookie[2..3] & 0x3fff)` (virtual slot `+0x308`), stores the cookie, goes to Initialized.
- Client restart rule: restart request is honoured only if an authorised cookie exists, more than 10 s since last client send, and (first restart packet, or >1.1 s since last restart packet, or >1.0 s since last NetConnection receive).
- Log strings present in the client: "Beginning restart handshake process.", "Ignoring restart handshake request, while already restarted (this is normal).", "Server sent restart handshake request, when we don't have an authorised cookie." The 4.25-only strings ("due to < 10 seconds since last handshake", "due to recent NetConnection packets", "Received unexpected post-connect handshake packet") are absent, so this is the 4.24 generation of the file.

**Conclusion (inferred from the above):** the handshake itself is not where the restart loop comes from. The client only sends non-handshake packets after it accepted the ack. A restart request is only sent by the server when a non-handshake packet arrives from an address that has **no connection**. So the server either never kept, or immediately closed, the connection it created after the challenge. The cause is in the first data packets, not in the handshake.

## 2. NetConnection packet header in the client

`UNetConnection::ReceivedPacket` is around `0x14435f200` (Ghidra splits it), `ReceivedRawPacket` at `0x1443621f0`.

- Reads `bHasPacketInfoPayload` and a jitter clock value with `SerializeInt(max 1024)` when `EngineNetVer >= 14` (binary: compare `0xd < EngineNetVer`, value `< 0x3ff`). **Same as stock 4.25** (`HISTORY_JITTER_IN_HEADER`).
- Bunch header reads `bControl`, `bOpen`, `bClose`, close reason (`SerializeInt` max 15 when `EngineNetVer >= 7`), `bIsReplicationPaused`, `bReliable`, channel index, etc. in stock order as far as read.
- Sequence mask `0x3fff` (MAX_PACKETID 16384). Same as stock.

So the stock 4.25 header reader should accept the client's packets. No patch indicated here.

## 3. AES handler in the client

Functions in `0x1409dcef0`–`0x1409ddb50`. Log strings are identical to stock 4.25 `AESHandlerComponent.cpp` (ECB): "received encrypted packet before key was set", "missing ciphertext", "failed to decrypt packet", "malformed packet, last byte was 0", "failed to write encryption bit", "failed to write termination bit". Layout presumed identical to stock (1 encryption bit per packet; when encrypted: termination bit, pad, AES-256-ECB). Not yet compared instruction by instruction.

Compared and found identical to stock 4.25: `UNetConnection::InitSequence` (client `0x14435b1e0`, virtual slot `+0x308`): packet ids from the two cookie words, reliable sequences `& 0x3ff`, gated by the randomize-sequence cvar.

## 4. Oodle handler in the client

Vtable `0x145c1ba58`: `Incoming 0x14119bc90`, `Outgoing 0x14119d2b0`, `Initialize 0x14119bf30`, `GetReservedPacketBits 0x14119bc80`.

- `Initialize` reads `[OodleHandlerComponent] bEnableOodle` from the engine ini (there is also a command-line force-enable flag, not used by our runner).
- With `bEnableOodle` false: reserved bits = 0 and `Incoming` reads nothing. **So disabling Oodle in the client's loose `Engine.ini` really removes it from the wire.** All three Wine prefixes on this machine carry that block.
- With it enabled (for a later "stock config" mode): 1 bit "compressed"; if set, `SerializeInt(DecompressedLength-1, max 1024)`, then the rest of the packet is passed to `OodleNetwork1UDP_Decode` with the dictionary (`Content/Oodle/FortniteGameInput.udic` / `Output.udic`). Supporting that on the server needs an Oodle Network v1 UDP decoder/encoder.

## 5. Control channel in the client (original server path is present)

`UWorld::NotifyControlMessage` is at `0x144695720` and contains the **server** cases.

- `NMT_Hello` (0): reads `uint8 IsLittleEndian`, `uint32 RemoteNetworkVersion`, `FString EncryptionToken`. Calls `FNetworkVersion::IsNetworkCompatible`; on mismatch sends `NMT_Upgrade` with the local version and closes. If the token is non-empty it calls the encryption-token delegate (and closes with "Encryption failure" if none is bound); otherwise sends the challenge. **Stock.**
- `NMT_Login` (5): reads `ClientResponse`, `RequestURL`, `FUniqueNetIdRepl`, `OnlinePlatformName`; logs "Login request: %s userId: %s platform: %s"; adds `SplitscreenCount`; calls `GameMode->PreLogin`. **Stock field order.**
- `NMT_Join` (9), `NMT_JoinSplit` (10), `NMT_Netspeed` (4): stock shape.

So through Welcome and Join the control channel is stock 4.25-compatible. Fortnite-specific behaviour enters only through the game mode overrides (`PreLogin 0x141950bd0`, `Login 0x141959680`, `GameWelcomePlayer 0x14195a020`, `PostLogin 0x141950f10`).

## 6. What this means for gate G3/G4

Everything on the wire from the first handshake packet to `NMT_Join` matches stock 4.25 except the float timestamp, which the patch already handles. **No further engine wire patch is indicated by the binary.** The restart loop therefore has a runtime cause on our server. The logs of the 2026-11-09 run were not kept, so it must be re-run once with logging; the decision tree below says what each outcome means and what to do. Run the server with `-FortniteOriginalClient -LogCmds="LogHandshake VeryVerbose, LogNet VeryVerbose, LogNetTraffic Verbose, LogFortniteNet Verbose"`.

| Look for in the server log (in this order) | Meaning | Action |
|---|---|---|
| No `Server accepting post-challenge connection from` | `HasPassedChallenge` failed although the ack was sent | Check `LastChallengeSuccessAddress` compare; check that the connectionless handler and the driver are the same instance the patch runs in |
| `Remote network version on first connection:` then `NMT_Upgrade`/close | Version path | Should not happen (`IsFortniteNetworkCompatible` returns true). If the line never appears, Hello was not parsed: go to the malformed row |
| `Encryption token received (...) but no usable key is configured` then close | Key not loaded in the server process | Start server and backend with the same `FORTNITE_NET_ENCRYPTION_KEY_FILE`; confirm the "returning a 32-byte key" line |
| `Malformed_Packet`, `Received packet with 0's in last byte`, `Packet failed PacketHandler processing`, bunch overflow errors, right after accept | Handler chain mismatch (one side has a bit the other lacks) | Print the server's handler component list at connection init. It must be exactly: StatelessConnect, then AES (encryption component). Confirm the AES plugin module actually loaded (`LogPacketHandler`), and that the client log shows `bEnableOodle` false |
| Accept, then nothing, then `UNetConnection::Tick: Connection TIMED OUT` | Client packets are being dropped as out-of-window | Log `InitSequence` on both sides (client: enable `LogNet Verbose` through the loose ini `[Core.Log]`); the two sides must print mirrored values |
| Server process restarts or asserts | A `check` in the patched handler | Read the callstack |

After the encryption ack, if packets fail to decrypt: compare the client's AES `Incoming`/`Outgoing` (`0x1409dd1c0`–`0x1409dd990`) with stock `AESHandlerComponent.cpp` instruction by instruction. That is the one part of the chain not yet verified at that level.
