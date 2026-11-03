# Two-field signed integer predicates

Run `python3 -I porting/behavior/bulk_two_field_predicate_runtime/prepare.py` from the repository. The scanner pins the original mapped image by SHA-256 and writes isolated proposals and complete decoded wrapper/leaf evidence to `/tmp/gameplay-bulk-two-field-predicate/`. `--output-dir` changes that destination; `--revalidate-own` includes entries promoted from this directory. It does not change the canonical manifest.

The finite accepted family is a complete standard parameterless Boolean exec wrapper calling a complete branch-free leaf that loads one same-object 32-bit integer, compares another against it and returns the signed or equality `SETcc` result. Both offsets must uniquely identify same-class reflected scalar `Int` fields and matching accessible generated `int32` declarations. The scanner validates reflection flags, the one-byte return record and class identity, and excludes all canonical pairs and existing class-qualified definitions. Other arithmetic, indirect access, control flow, field kinds, and inherited fields are deliberately outside this scanner. Existing different proposal bodies abort the scan.

These are source-recovery proposals only; no build or runtime parity is implied.
