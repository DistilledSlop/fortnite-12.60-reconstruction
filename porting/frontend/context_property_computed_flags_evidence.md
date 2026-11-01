# Original constructed property flags

The three loaded GetContext property checks in context phase2 failed because
the diagnostic compared raw FPropertyParams flags with constructed property
flags. Both original code and current Engine constructors add the same exact
type bits: **0008001040000200**, representing IsPlainOldData, NoDestructor,
ZeroConstructor and HasGetValueTypeHash. These bits must be added to the raw
expectation; unknown bits must still fail equality.

The late original image hash is
`338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`,
with VA140000000+fileoffset. GetContext singleton140823490 calls
ConstructUFunction142ec43b0, which calls the property producer142ec2160.
Its kind table142ec3c38 selects Class11→142ec2e8b and Object12→142ec2b2f.
Both cases load raw flags at descriptor+10, OR the exact constant, pass the
result in the Win64 stack argument and store it at constructed property+40
through constructor142e75a40. The replay verifies25 original windows, including
all three raw descriptors, names, offsets and kinds.

The expected constructed flags are:

| Property | Raw flags | Constructed flags |
| --- | --- | --- |
| ReturnValue | 0010000000000580 | 0018001040000780 |
| Class | 0014000000000080 | 001c001040000280 |
| ContextObject | 0010000000000080 | 0018001040000280 |

This proves static original constructor behavior, not an original live heap
measurement. Current Engine UnrealType.h independently computes and ORs these
bits in TProperty, with FObjectProperty/FClassProperty using that path. The v6
diagnostic measures the actual loaded flags, dimensions, offsets and type
identities; fresh runtime acceptance still requires every exact check to pass.
The raw original UHT/object/linked ELF checks remain unchanged.

Frozen local proof: `/tmp/native-context-computed-flags-proof-1/receipt.json`,
SHA-256 `5205b8f3cb5b600ad5ed1039ca5019127a4de6dd375d35a6bb6243cafaf70ef1`.
Replay uses only the standard library and reads a completed image:

```sh
python3 -I porting/frontend/inspect_context_property_computed_flags.py \
  --image /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin \
  --output /tmp/fresh-original-computed-flags.json
```

Phase2 failure and its root integrity audit remain preserved; no prior run is
reclassified as successful by correcting the diagnostic expectation.
