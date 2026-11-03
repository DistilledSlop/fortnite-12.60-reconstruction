# Pawn state and squad notification evidence

Original runtime image `porting/generated/dump/FortniteClient-Win64-Shipping.runtime.exe`, SHA-256 `5f056080237b5a0fdab284700d5bd975acf8c30ffeeb37472f5ba5542fa2dc38`. Reflection `fortnite_game_reflection.json` SHA-256 `c11cc036439fd2832514681e1bac12c933ee78b6a351f36d44dabf170d8eab1b`; decoded `bool_bits.json` SHA-256 `c65e8d4abc52683c7a4e646e846f22941d63a36aa493985fd6ac16c1f07b3e99`.

Addresses below are original image VAs. Offset/mask mappings are from reflection property SetBitFunc machine code, not the Ghidra class layout. The `mov8` setters write the entire byte, and the generated declarations use native `bool`; `or8` setters represent packed flags.

- `AFortPawn::IsStaggered` exec `0x142a1cdb0`: tests `+0x520 & 0x80`, nonzero byte `+0x618`, then `+0x520 & 0x40`, returning their OR. Setters `0x142a18170`, `0x14274f000`, `0x142a18150` respectively map these to `bIsStaggered` (or8 0x80), `bIsStunned` (mov8), `bIsKnockedback` (or8 0x40).
- `AFortPlayerPawn::HasRespawnedInAir` exec `0x142a7cce0`: tests both `+0x10e9 & 0x80` and `+0x10ea & 0x01`. Setters `0x142a76430` and `0x142a76420` identify `bIsRespawning` and `bIsRespawningInAir`, both or8.
- `AFortPlayerPawn::IsParachuteLockedOpen` exec `0x142a7d3b0`: copies whole byte `+0x2285`; setter `0x142a76540` is mov8 for `bLocalParachuteLockedOpen`.
- `AFortPlayerPawn::IsParachuteOpen` exec `0x142a7d3d0`: copies whole byte `+0x10e6`; setter `0x142a763e0` is mov8 for `bIsParachuteOpen`.
- `AFortPlayerStateAthena::Server_TogglePreserveSquad_Implementation` `0x141a59530`: compares whole byte `+0x13e0` to zero and stores the resulting 0 or 1 there; setter `0x142a8b4e0` is mov8 for native bool `bPreserveSquad`.
- `AFortPlayerPawn::IsInCharacterCustomization` exec `0x142a7d1a0`: reads dword `+0x2004`, shifts right three and masks one, equivalent to testing bit 0x08. The mapped reflected fields at that offset use masks 0x02 and 0x40; no property maps bit 0x08. Its named backing field is unresolved, so this function remains skipped.

The whole-byte reads are modeled as native bool values, which retain the original behavior for valid native bool representations (0 and 1). This does not claim equivalence for corrupted or noncanonical raw bytes.

`AFortPlayerStateAthena::ClientReportTeamKill_Implementation` at `0x141a3d8f0` reads Actor Owner at +0xe0, tests its type against `AFortPlayerControllerAthena::StaticClass` at `0x142a53450`, calls accessor `0x1419e8cd0` to obtain controller +0x31f8, and broadcasts the signed int32 argument through `0x140f2c470`. The latter has the UE native multicast reverse iteration, locking and compaction shape. The new `TeamKillsReportedNative` name is reconstructed; the contract is medium confidence. This is distinct from the no-argument controller multicast at +0x3210 used by `OnRep_TeamKillScore`. The primary review found no reflected property at +0x31f8 in the controller class family. Exact image-byte SHA-256 values: RPC 0x58 bytes `3282737b92266e16f74ef6d1adb4742db109b49153f1937298349209e06b4da2`; accessor 8 bytes `9d1b1464be8da9869402cbbe2d7b00c7458a0d7f07b07014faed822033dfb150`; broadcast 0xa4 bytes `e9a656d4c9eb4a6198e0f5a3dc40adf34cdab041baa464fc161c179ccb02d25f`. Review disassembly is in `/tmp/gameplay-team-kill-review-1/`. These are source-level findings; gameplay lifecycle and runtime integration remain unverified.

Authoritative raw image: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`, SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`; VA is read at file offset VA - `0x140000000`. All body and setter windows below match the older PE dump byte for byte. The hash is of the indicated window, not the entire function unless stated:

| VA | bytes | SHA-256 |
|---|---:|---|
| 0x142a1cdb0 | 0x37 | bfd66eb91b97948a628dba7a2af566e3e17c274f4794ed86eab34c9bc70c3c62 |
| 0x142a7cce0 | 0x31 | fcd1a7f818dc833559df3456b46aa58f620b234378bd4da30864a52c55116a78 |
| 0x142a7d1a0 | 0x24 | d3409539117d260f2604612e4f9b3c57f18377654a76b8e6deb1f28a88653d54 |
| 0x142a7d3b0 | 0x20 | b36ea885cd5ec0eca9c69f19364b9587d57a35d61bf1cbe86566d892324c3fd2 |
| 0x142a7d3d0 | 0x20 | f8e9cfc31762eb5d6e3ec8c2b78df177052984496707ed52280f72ba39328687 |
| 0x141a59530 | 0x11 | 075628bb0b6d52e292e10e72b43a4815a048c9be269da286b10c3af78922f2b3 |
| 0x142a18170 | 8 | b196bbb3292c8ecf0b3f23bd883eb112ea08acdbcfa89c3444cb1e16d5cc5cfa |
| 0x14274f000 | 8 | 36108ca380d4e09d89b7ffa9afb05d1e40e3baa85fd7e54e23a5c10f0c238805 |
| 0x142a18150 | 8 | 60f4e8c2d1742426c8cdb0092054d696c08204fc1fc541573639fa50c892533e |
| 0x142a76430 | 8 | 9c07ec9cfcb538bababfe3408df17c2ce36f24670f2c82a5caec7371dad37d33 |
| 0x142a76420 | 8 | d287b56cdff79b4355e37b1b9cf48e69d3a039c985965c6b5089a45756e50608 |
| 0x142a76540 | 8 | 690e7651a2e3c21600effc57c196df69d97d3a7286c1a5aba0e98ef0e9948dab |
| 0x142a763e0 | 8 | ec478c92674c89651c21a9e2d81e20b052892da20ec726beec9086d9eacf9709 |
| 0x142a8b4e0 | 8 | ce08bdb606ade5d54dd7025f3b6ea4d536e704a71c92b7e0b30cdc24b39bd4fc |

The three team-kill windows and hashes are given above and also match both images.
