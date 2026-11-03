# Leaf raw-storage assessment

`python3 porting/behavior/leaf_storage83_runtime/prepare.py` assesses the 12 unresolved constant-write leaves in the batch83 scanner report, without rereading the pinned image. It uses Bool `size_of_outer` agreement for class and immediate parent bounds, checks the parent and class reflected and canonical-field occupancy with the conservative `bulk_mutation_storage_runtime` interval rules, and rejects unknown occupancy and aggregates that may extend into the candidate range. Only byte MOV and dword OR are considered. Evidence and reasons are recorded in `report.json`.

One proposed field and body: `UEnableMultiFactorModal::AnimationsComplete`, byte at `0x44b`. The parent ends at `0x448`; the class ends at `0x4b8`. Closest known class fields occupy `0x448` and `0x449`; no reflected or canonical occupancy covers `0x44b`. The leaf writes byte 1; the neutral raw byte declaration asserts no bool meaning, default, lifetime or unrelated writer behavior. Review the source evidence and current canonical occupancy before promotion. Image SHA-256 is inherited from the earlier pinned source; this run did not reverify it. No generation, tests, builds or probes were run.

Root reviewed and promoted the proposed bodies/fields in generation83: final
2,290 accepted entries,0 rejected. This confirms source generation only; native
build/runtime validation is deferred to checkpoint3,040.
