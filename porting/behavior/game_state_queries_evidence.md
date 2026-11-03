# Game-state count, playlist-name and focal-point bodies

Authoritative input: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, VA minus `0x140000000`, image SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`. Existing Ghidra decompilation/disassembly is under `generated/callee_probe/blocking/`; runtime Windows layouts below map to named fields, never hard-coded Linux offsets.

| Native body | Bytes | SHA-256 |
|---|---:|---|
| 0x141991ce0 | 0x2f | c771ea61e25a9c4738ce62b1a16d01ddfdbd5166487cb48697d376e12636b63f |
| 0x1419964d0 | 0x1d | 60f38de7a8dd2ad385f94e20a5531d31ed0b0070f5f0db6ae5df7107d93aa7f0 |
| 0x1419997b0 | 0x1d | 7c78beabb9f6d4b56129c09cc332fdaea718019b4747904f6071f4275ad8888f |
| 0x1419ae6f0 | 0x3e | faffe4dd4986516d8d325d0897e22465d87cae6ebafc9aafeb53463750bef71e |

`GetCurrentPlaylistName` body selects override +0x1df8 else base +0x1df0, reads UFortPlaylist.PlaylistName at+0x34, and returns eight zero bytes when neither exists. The Athena-owned reflection candidate (outer 0x14290e020) returns FName with flags0x4020403; the same-named Text-returning const candidate belongs to another outer and must not set this signature.

`GetPlayersLeft` writes Humans from PlayersLeft(+0x12f0), Bots from PlayerBotsLeft(+0x1ecc), and Total as their sum. `GetTotalPlayers` uses TotalPlayers(+0x12ec) and TotalPlayersBots(+0x1ec8). Struct blocks0x147a5dde0 /0x147a5dd20 identify FPlayersLeft /FTotalPlayers, size12, Humans0/Bots4/Total8. Both bodies write every field with no calls.

`SetFocalPointActor` first tests original Role byte+0xf0==3. On authority it writes FVector payload to FocalPointOffset(+0x1328), InDuration to RemainingFocalPointActorDuration(+0x1318), InFocalPointFOV to FocalPointFOV(+0x1334), and InFocalPointActor to FocalPointActor(+0x1320). Other roles return without mutation. There are no delegate, OnRep or other calls. The portable body uses GetLocalRole()/ROLE_Authority and named reflected fields; the original FVector payload assignment is preserved by FVector_NetQuantize conversion.

The count fields' original flags `0x10000000002014` include BlueprintVisible (0x4), BlueprintReadOnly (0x10), and Transient (0x2000). Their use as original BlueprintPure return values establishes BlueprintType support. The generator now validates their exact identities/layouts/fields before emitting USTRUCT(BlueprintType) and UPROPERTY(BlueprintReadOnly, Transient); it does not make all structs Blueprint types.

Scratch generation passes265 splices/zero rejections. The four new entries pass focused source checks; fresh UHT and syntax compilation of both the generated game-state TU and its UHT reflection TU pass. Artifacts/argv are in `/tmp/gameplay-state-query-validation-2/` (see HANDOFF). Shared Native promotion, client/server linking and actual gameplay runtime remain pending. No runtime or milestone completion inferred.
