# Native scalar storage proposals

Run from repository root:

```sh
python3 -I porting/behavior/bulk_native_scalar_runtime/prepare.py
```

Requires Capstone in `/tmp/common-input-capstone` and the mapped original image
`/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`;
the script checks SHA-256 `338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`.
It writes manifest proposals and evidence to `/tmp/gameplay-bulk-native-scalar/`,
and guarded `.inc` bodies here. `--output-dir PATH` changes the report directory;
`--revalidate-own` includes canonical bodies from this directory and requires
canonical fields with `recovery_source: bulk_native_scalar_runtime` to agree.
Other canonical ported entries remain excluded.

Complete inline or wrapper-to-complete-pure-leaf plain byte and dword reads are
matched with the reflected Bool, Int or Float return and the scalar matcher's
flags and ABI rules. Each offset must lie within the unique declaring class's
original boundaries, obtained using the generator’s property grouping and agreeing positive top-level Bool
`size_of_outer` records for both class and original parent. The selected class
must have a generated declaration with that same parent. Reflected storage,
including Bool byte offsets, and canonical non-reflected fields are reserved;
classes with unresolved property grouping are rejected;
unknown widths reserve through the next known offset or class end. Proposed
intervals must agree in kind and width and must not overlap. No masks, constants,
pointers, guessed sizes, or impure calls are accepted.

The generated declaration orders fields by original offset but does not enforce
original binary layout. These are source-level storage proposals only: the
original writers, constructor and lifecycle are still unported. No default is
invented, and these entries do not establish runtime parity or merit class
feature credit. Generator validation, compilation and integration remain
necessary before promotion.

The corrected property-grouping replay (`--revalidate-own --output-dir
/tmp/gameplay-bulk-native-scalar-corrected`) reproduced all 31 getters and 31
fields with identical manifest entries.
