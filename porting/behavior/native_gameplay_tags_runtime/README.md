# Shared native gameplay tag initialization

Canonical support-source bundle, ready for generator integration. No generated
Native/build files were changed and no runtime compatibility is claimed.
`source_policy.sources()` returns the exact header, implementation and registration
include to emit together in module `FortniteGame`; `DEPENDENCIES` names `Core` and
`GameplayTags`. Authorize the emitted support header in the behavior include gate.
Include `FortNativeGameplayTagsRuntime.h` and call
`FFortNativeGameplayTagsRuntime::Get()` from **every recovered consumer** of
original accessor `0x141ed1800`. Do not add startup calls or separate query statics.
The implementation exports `FORTNITEGAME_API` for consumers in other modules.

The public named getters are `AthenaCreative`, `AthenaCreativeMM`,
`AthenaCreativeLTM`, `AthenaCreativeUtilitySpawning`, `AthenaBattleLab`,
`GameplayActionPlayerChangeEquipment`, `EventWeaponsWeaponSwitch` and
`AthenaHideForCapture`. They return the original stored registration result;
`GetTagAtOriginalOffset` covers all recovered scalar tag fields. The source type
is not a binary object overlay. Map/container/set getters expose the other
recovered state without guessing reflected field names. No UObject/reflection
class is required.

## Complete constructor effects

The whole `0x141ebb310` constructor through its first return `0x141ec64d8` is
closed. Its only conditional control flow obtains the gameplay tag manager if
its global is null. All remaining calls are resolved below. Recovery finds:

- 1,136 unique scalar tag fields registered, in exact original execution order.
  This includes the 61 registrations inside helper `0x141ec8c00`.
- 14 byte-to-tag movement map entries: two at original `+0x880` through
  `0x141ec9870`, twelve at `+0x8d0` through `0x141ec87c0`. Their shared helper
  `0x140d756c0` copies a byte key and eight-byte tag value into 20-byte set nodes,
  hashes the byte key, and returns the mapped tag, matching `TMap<uint8,FGameplayTag>`.
- Each registration's producer `0x141ec98b0` obtains
  `UGameplayTagsManager::Get()`, constructs `FName` from the ASCII literal with
  add semantics, calls `AddNativeGameplayTag` with the exact `(Native)` comment,
  and stores the returned tag. It then compares the first 11 ASCII bytes with
  `GameplayCue`, case-insensitively (`0x142bcf840`), and inserts matching returned
  tags into the set at `+0x20f8`. There are 32 matching registrations. The set
  insertion `0x1406a8a80` copies/hashes/compares an eight-byte tag payload in
  16-byte nodes; it does not store a name-to-tag map. Invalid returned tags are
  inserted too, with normal duplicate collapse.
- Helper `0x141ec8c00` then fills `TMap<FGameplayTag,int32>` at `+0x2058` with
  20 entries and `+0x20a8` with 17 entries. Its insertion `0x141cf6b40` copies
  the tag key and 32-bit value, hashes both tag words and uses 20-byte nodes.
- After **all** registrations and map entries, the constructor itself calls
  `UGameplayTagsManager::Get().DoneAddingNativeTags()` at `0x141ec643b`.
  Body `0x143e8dfa0` checks `GEngine` and the done byte, broadcasts last-chance,
  sets done, destroys/reconstructs the tag tree and broadcasts completion,
  matching the checked-in UE 4.25 source. This is an explicit constructor effect,
  in addition to the engine's PostEngineInit delegate. Its `GEngine` guard is
  preserved by calling the engine API, including when no engine exists yet.
- Finally seven `FGameplayTagContainer::AddTag` calls (`0x143e8b3b0`) populate
  `+0x1b10` from fields `0x440,0x418,0x450` and `+0x1b30` from
  `0x2a8,0x2c0,0x2b0,0x2c8`, in that order. Parent-tag effects come from the
  real engine container operation after the original finalization call.

All scalar tag fields start invalid. The three scalar slots `+0x14c8`,
`+0x22b8`, `+0x2418` remain invalid: no registration or destructor allocation
uses them. The five TMap/TSet headers initialize to normal empty UE containers
(including inline bit-array capacity128 and free-list index-1), and both tag
containers initialize empty. No allocation/reserve is added before registration.

The original accessor uses MSVC thread-safe local-static guard machinery at
`0x148132cd8`, returns singleton `0x148130770`, constructs once, registers
`0x145865cb0` with atexit, and completes its initialization guard. The source
uses one C++ function-local static. A compile-time gate rejects compilers with
thread-safe statics disabled. The registered destructor releases the cue set,
second and first tag/int maps, second and first tag containers, custom movement
map, movement map, in that order. Source member declaration order preserves that
reverse destruction order; no destructor invokes tag-manager unregistration.

For the five playlist helpers, null must return false before `Get()`. For a
nonnull playlist use its actual `GameplayTagContainer.HasTag()` with the stored
getter, and append the original CreativeMode bit check only for CreativeMode.
All other original singleton consumers must use this same `Get()` owner. The
concrete `0x144d6f690` consumer's `+0x24c0` is `Athena.HideForCapture`, now exposed.

## Evidence and focused verification

Authoritative image: `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
VA minus `0x140000000`, SHA-256
`338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`.
`constructor_events.json` records every registration literal VA/call/field and
all subsequent map/finalization/container effects. `recover_constructor.py`
linearly interprets the complete constructor and its registration-list helper,
rejects unknown instructions/callees/control flow, checks the authoritative
image hash and writes a fresh event stream plus all1,192 initialization stores.
It does not call Ghidra or modify the binary. `source_policy.py` pins the recovered
event stream SHA-256, full effect counts/unique ownership and finalization order,
and rejects divergence between the event stream and emitted source include.

| Body | Size | SHA-256 |
| --- | --- | --- |
| constructor141ebb310 | b1c9 | d1929bb89655ecf4aad73aa6c9f2248eaccf7dc79b37be7a83d280fdae5af0d9 |
| accessor141ed1800 | 7b | 0ab2f937d365430f5e0432cb2b801fe6a078f36672f24e86632ba7be68d069c2 |
| producer141ec98b0 | f2 | ef0f23c679d5d181bebe9825fe646043560c54217e59a110298f448059ccf045 |
| list/helper141ec8c00 | c68 | 7027b4f2de1bf1d080b85c2e7a9c57ec2351674eb2d29f6e4b7346a25375c569 |
| custom movement141ec87c0 | 39 | 81d75e73caa5bcc0dca4a9c0992915851d5bbe821ca1b43076d29eea74153e7a |
| movement141ec9870 | 39 | a102a4dd1d5d49e1c47ef438616b2055bfc538f4dbf1d70c897d57aab6f8afbc |
| DoneAddingNativeTags143e8dfa0 | 58 | 4a751ff934c10e30ec88bf356edc807c6f633d30c1415fa09bfe32ce5e29ee79 |
| registered destructor145865cb0 | 100 | 7339d8e88625d8a77cf6bf0580b95253e50b0945e62e1b7e8cad75c64282a160 |

Exact commands, run from repository root:

```sh
PYTHONPATH=/tmp/common-input-capstone python3 porting/behavior/native_gameplay_tags_runtime/recover_constructor.py /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin --output /tmp/gameplay-native-tags/recovered
cmp porting/behavior/native_gameplay_tags_runtime/constructor_events.json /tmp/gameplay-native-tags/recovered/constructor_events.json
python3 -I porting/behavior/native_gameplay_tags_runtime/source_policy.py
```

PASS: fresh authoritative recovery matches the canonical stream byte-for-byte;
1,181 effects (1,136 registrations,37 map insertions,1 finalization,7 container
adds),1,192 initialization stores. Source policy gate passes and returns all
three required sources. No per-tag/body tests, UE build, shared promotion or
runtime run was performed; primary integration owns the combined compile and
normal gameplay validation. Temporary bounded disassembly windows and recovery
output are in `/tmp/gameplay-native-tags/`.
