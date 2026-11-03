# Class registration sizes

The original runtime image at `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin` (image base `0x140000000`, previously pinned SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`) contains these calls to `GetPrivateStaticClassBody` (`0x142d698b0`). The digest was not recomputed for this bounded inspection. Run `python3 porting/behavior/class_size84_runtime/verify.py` from the repository root to recheck the specific bytes and class mappings.

| Class | FClassParams | class_noregister | registration builder | call | InSize | InAlign |
|---|---:|---:|---:|---:|---:|---:|
| UFortGenericRecord | 0x1463f6140 | 0x14287cce0 | 0x14292e780 | 0x14292e817 | 0xf0 | 0x8 |
| AFortPhysicsPawn | 0x14648e670 | 0x1428394a0 | 0x142a25dd0 | 0x142a25e6e | 0x380 | 0x10 |
| AFortGameState_InGame | 0x1463f1e00 | 0x142926770 | 0x142926770 | 0x14292680e | 0x6b8 | 0x8 |

The class-name map evidence links each block to its name; reflection links the block to its class_noregister function. Two no-register functions jump to the builders; the third is itself the builder. Each builder passes the same UTF-16 class name, skipping its initial A or U, in RDX. The documented helper signature in `porting/ghidra/name_tables/helpers.json` places `InSize` fifth and `InAlign` sixth. The Windows x64 ABI passes these at caller `[rsp+0x20]` and `[rsp+0x28]`, respectively. The builders directly store the tabulated immediates there immediately before the call. The script validates the instruction bytes, relative targets, literals and mappings; sizes are not inferred from property offsets.

Generated headers identify AFortPhysicsPawn and AFortGameState_InGame as the immediate parents of AFortAthenaVehicle and AFortGameStateZone, respectively. These sizes resolve their missing-parent-size evidence, but do not by themselves establish that a particular getter's storage is unoccupied or otherwise safe to implement.
