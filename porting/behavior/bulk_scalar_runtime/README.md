# Bulk scalar getter proposals

Run from the repository root:

```
python3 -I porting/behavior/bulk_scalar_runtime/prepare.py
```

The script requires Capstone at `/tmp/common-input-capstone` and the original
mapped image at `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`.
It loads and hashes the image once (SHA-256
`338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`),
reads the complete native function inventory, and writes proposed bodies here
and a report plus manifest entries under `/tmp/gameplay-bulk-scalar/`. The
canonical manifest determines which ported functions are excluded. The script
never edits the canonical manifest or generated Native code and refuses to
overwrite an existing body with different content.

To rescan entries already ported from this directory while still excluding
other ported entries, and write the results separately, run:

```
python3 -I porting/behavior/bulk_scalar_runtime/prepare.py --revalidate-own --output-dir /tmp/gameplay-bulk-scalar-replay
```

The matcher accepts only exact, complete recognized frame-inline sequences or
an exact frame wrapper calling an exact leaf getter. It checks the native flags,
zero-argument reflected return, selected target class and package, and a unique
same-class reflected scalar field at the accessed offset. It checks the exact
bool mask, rejects bit masks whose raw result is not normalized to 0/1, and
accepts shifted bool getters only when the reflected mask equals `1 << shift`.
It also recognizes complete constant returns with matching output widths and
reflected types; floating constants must be finite. It only emits `Int`, `Float`,
untyped `Byte`, and `Bool` returns. Enum, pointer, inherited-field, unusual code,
virtual calls, unknown flags, RPCs, reference returns and unrecognized ABI forms
are deliberately unsupported. Decompiler C was not available as a whole-inventory
indexed input for this pass; the complete machine-code patterns provide the
evidence here.

The original first pass found 131 proposals from 23,758 inventory records; the
second found 84 additional proposals: 45 shifted bool getters, 32 constant bool
returns, six constant int returns, and one constant float return. Counts depend
on the canonical manifest and selected options. Proposals require normal
generator, compilation, and integration validation; they do not establish an
end-to-end feature or runtime parity.

All 215 proposals are canonical and spliced. `--revalidate-own` reproduced exactly
215 canonical entries with no body mismatch. Combined client/server builds and the Apollo join pass;
receipts are in `../../generated/gameplay-bulk-integration-1/`.
