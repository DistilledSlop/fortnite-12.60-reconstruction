# Handoff

Six independent native bodies. Exec evidence is `porting/generated/decomp/<class>/<function>.c`; native callee evidence is `porting/generated/callee_probe/blocking/<native address>.{c,asm,json}`. Signatures are proposed; output argument names are source choices. Reflection-generated fields are in `porting/native/Source/FortniteGame/Generated/`.

- AFortPlayerPawn::SetInSlipperyMovement: exec `0x142a80170`, native `0x14234d620`. Bit 0x80 of +0x10e7 maps to bIsInSlipperyMovement; conditional write has no observable side effects.
- AFortPlayerPawn::IsSkydivingFromLaunchPad: exec `0x142a7d4b0`, native `0x142331fc0`. Original tests +0x10e7 bits 0x01, 0x04, and 0x08.
- AFortPlayerPawn::SetNumActiveBalloons: exec `0x142a80450`, native `0x14234dfa0`. Original clamps signed input to 0..255 and stores byte at +0x10e3.
- AFortPlayerPawn::GetWaterImmersionDepth: exec `0x142a7cc20`, native `0x142325930`. Tests CurrentWaterBody +0x22e8; reads movement component +0x1d08 without null check.
- AFortPlayerPawn::GetLastWaterSurfaceInfo: exec `0x142a7c3b0`, native `0x14231f2a0`. Reads the two vectors at movement component offsets +0x1d3c and +0x1d48 in argument order; no null check.
- AFortPlayerControllerAthena::GetPlaysetSize: exec `0x142a58830`, native `0x1419d5f80`. Uses CurrentPlayset +0x3588 and size fields +0x3f8, +0x3fc, +0x400; defaults to 1 and clamps only Z to at least 1.

No build or runtime tests performed. BalloonActiveCount offset 4323 is recorded in `porting/generated/server_authority_inventory.json`; playset class block 0x1464dff40 in `porting/generated/reflection/fortnite_game_reflection.json` identifies SizeX/Y/Z at offsets 1016/1020/1024. Bool masks are in `porting/generated/reflection/bool_bits.json`. Native whole-body windows are pinned in `native-evidence.json`.
