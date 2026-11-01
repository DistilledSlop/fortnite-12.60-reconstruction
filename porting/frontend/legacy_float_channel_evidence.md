# Original float-channel version and Splash animation keys

After generated native commit `7c0aca3`, Login passes the preceding missing
Blueprint-super declarations and fails in `FMovieSceneFloatChannel::Serialize`
at `MovieSceneFloatChannel.cpp:1099`. The frozen checkpoint is
`generated/material_pipeline/frontend-super-events-checkpoint-results.json`,
SHA-256 `dde521a1bd2d1e1ceb53ec2bc7a3f7f991da623620fa2cec04a2c193f6bf62f1`.
Its Splash package hash receipt confirms that the staged header and payload
are byte-identical to the extracted originals.

The failing `30645 / 30641` positions use the combined package address space:
15,958 header bytes plus 14,683 payload bytes. They do not describe a converted
or truncated 30,641-byte physical `.uexp`.

## Original code identifies version 11

The pinned original runtime memory image uses VA = `0x140000000 + file offset`.
The exact image SHA-256 is recorded by the reproduction helper.

- Static registration at `140476eb0` loads the Sequencer GUID from
  `147c39c78`, the UTF-16 literal `Dev-Sequencer` from `146029028`, and
  immediate version **11** (`BA 0B 00 00 00` at `140476ec2`) before calling
  registration `142ccf680`. The original version-container initialization
  independently supplies 11 for the same GUID at `141f3c621`.
- `MovieSceneFloatChannel` original struct metadata identifies its native
  operations vtable `1467cfbf0`. Slot 9 points to `143b50e70`, which adjusts
  the data argument and tail-jumps to `143b40790`. Its exact 30-byte body
  registers use of the Sequencer GUID and returns false without consuming
  channel data. This requests ordinary reflected property serialization.
  Slot numbering here comes from the actual original table; overloaded
  virtual methods must not be assigned slots from a different compiler ABI.
- Original float-value serializer `143b407b0` and tangent serializer
  `143b40900` test custom version 10, then use the older field order. A
  float value contains Value, InterpMode, TangentMode and 17 tangent bytes:
  **23 serialized bytes**, distinct from its 24-byte native memory size.

Stock UE 4.25 adds `SerializeFloatChannelCompletely` as Sequencer version 12.
For an unversioned package without custom-version entries,
`FPackageFileSummary` substitutes the current registered versions. That makes
the current float-channel code choose the complete native format even though
the original game registered version 11. CUE4Parse's UE4.25 fallback to 12 is
therefore not primary evidence of this game's channel format. Its separate
Fortnite ShowCurve version 52 is not the cause established here.

## Original package preserves three keys

The original summary contains zero custom-version records and zero engine/
licensee versions. Header-only export inspection and independent raw export
table reads agree on export 22, `MovieSceneFloatSection_0`, logical offset
22,586 and size 659. Its `FloatCurve` struct property starts its data at
22,635 and declares 261 bytes. Those bytes contain:

| Property | Array count | Serialized element bytes | Values |
| --- | ---: | ---: | --- |
| Times | 3 | 4 | 0, 30000, 60000 |
| Values | 3 | 23 | 1.0, 0.0, 1.0 |

Both arrays retain their original inner struct tags. Every float value uses
InterpMode 2, TangentMode 0, WeightMode 0, with all tangents and weights zero.
The channel ends with the original `None` tag at its exact declared boundary.
The helper retains all original key bytes alongside decoded fields.

The first bytes are the `Times` property FName, index 294. Misreading them as
the new native format produces extrapolation bytes 38/1, element size zero
and array count **1,310,720**. This accounts for entering the Times fallback
array path and overrunning the package. Clamping that count would discard
the animation and leave the reader misaligned.

## Reproduction and queued reader change

```sh
python3 -I porting/frontend/test_legacy_float_channel.py
python3 -I porting/frontend/inspect_legacy_float_channel.py \
  --output porting/generated/frontend-native-evidence/movie-scene-float-channel-proof-NEW
```

All eight tests pass, including the actual original package and rejection of
truncated frames, count mismatches, changed element size, invalid names and
trailing bytes. Authoritative proof:
`generated/frontend-native-evidence/movie-scene-float-channel-proof-2/receipt.json`,
SHA-256 `fb652f574d2aa1335a9db7dcd5c33af2f2fd3b270498acafa1268ec732593bec`.
It includes a frozen helper source and exact original bounded code/assembly.
The first development proof is retained with a separate frozen-tool binding.
No Ghidra or original game execution was used for this evidence.

The **unapplied** patch
`patches/ue425-legacy-float-channel-version.patch` passes `git apply --check`
against the Engine checkout. With explicit `-FortniteLegacySequencer`, it
selects source version 11 for loading float-channel/value/tangent serializers
whose linker identifies an unversioned source package. This retains tagged
channel parsing and the 23-byte values together. It does not mutate the
global version registry or summary, and preserves existing save and ordinary
versioned-package paths. Current matched Paper2D cook headers explicitly
record UE version 518 and custom versions, so they do not meet this gate.

The sole build owner must apply it after the current Keybind source freeze,
compile both targets, repeat Login with and without the flag, and inspect the
actual loaded section's three times, values, interpolation modes and tangents
against this source proof. Root/Login probes, existing material regressions
and fresh joins must remain recorded. Portable parser tests and an applicable
patch establish no runtime loading, animation playback, widget integration or
original visual parity; all four product milestones remain open.
