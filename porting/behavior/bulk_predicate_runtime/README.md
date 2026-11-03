# Bulk object null-predicate proposals

Run `python3 porting/behavior/bulk_predicate_runtime/prepare.py` to verify the
pinned original image and scan the native inventory. Bodies are written here;
the proposal manifest and instruction evidence are written under
`/tmp/gameplay-bulk-predicate/`. Use `--output-dir PATH` for another report
destination and `--revalidate-own` to include already ported entries from this
directory. Differing existing bodies stop the scan.

A complete thunk or complete wrapper and leaf must compare a qword field to zero,
produce a normalized boolean with `sete` or `setne`, and store exactly one byte.
The field must be a unique same-class reflected plain Object property whose
non-interface pointer type agrees with the generator and generated declaration.
Only zero-argument reflected bool returns and permitted native instance flags
qualify. All other code and property forms remain excluded. The proposals are
source recovery only and require integration and validation.
