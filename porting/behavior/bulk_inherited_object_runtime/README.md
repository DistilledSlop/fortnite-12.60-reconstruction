# Inherited object pointer getter proposals

Run `python3 porting/behavior/bulk_inherited_object_runtime/prepare.py` to scan the
pinned runtime image and emit isolated bodies here and the manifest and evidence
in `/tmp/gameplay-bulk-inherited-object/`. `--output-dir PATH` selects the report
directory; `--revalidate-own` includes canonical entries from this directory.
Existing differing bodies cause the scan to stop.

A candidate needs the complete object getter wrapper and leaf (or complete inline
thunk), exact reflected Object return and field type references, a unique field
in the supported original ancestry, and agreement between each selected
FortniteGame/FortniteUI generated C++ base and its original reflected parent.
Only accessible generated raw pointer fields and resolved non-interface target
classes qualify. Return expressions qualify the declaring ancestor to avoid name
hiding. Engine ancestors, class, soft, weak, interface and owning wrapper types
are not inferred. Canonical ported entries, maintained definitions and generated
non-stub definitions are excluded. Generated declarations, signatures, flags,
shape and constness must agree. This is source evidence, not runtime validation.

On 2026-11-03 the command above scanned 23,758 records and yielded **zero**
proposals. Thirteen complete code matches remained: seven lack an unambiguous
matching reflected field within the supported chain, and six lack a supported
ancestry. Their names, offsets, instructions and available chain are recorded in
`/tmp/gameplay-bulk-inherited-object/report.json`. No canonical or Native files
were changed, and no build or runtime tests were run for this scan.
