# Constant-write leaf scanner

Run `python3 porting/behavior/bulk_leaf_mutation83_runtime/scan.py`. It scans selected reflected no-argument native execs not already ported, recognizing only either established complete FFrame-advance sequence followed by a direct unconditional jump to a leaf consisting of exactly one constant byte/dword write or byte mask operation and RET. No authority-guarded leaves are recognized. Both wrapper and full leaf bytes and SHA-256 are recorded in `report.json`. Mapped image identity is pinned to the previously established SHA-256 but is not rehashed by this scan.

Only existing accessible reflected or canonical scalar declarations with matching offset, width and operation are proposed. Packed bool operations additionally require a unique matching reflection mask and generated bitfield declaration; whole-byte bool stores require an ordinary bool declaration. Excludes canonical and batch82 pending entries, enforces class and parent declaration, function flags and signature, and limits proposals to 20. Results are evidence for review, not proof of build or runtime parity. It creates no storage and makes no canonical edits.

Scan result: 22 matching leaf instruction shapes, 10 proposals. Review exact reflected layout, generated declarations, and raw instruction evidence before promotion. No generation, build, or tests were run.

Root reviewed and promoted the proposed bodies/fields in generation83: final
2,290 accepted entries,0 rejected. This confirms source generation only; native
build/runtime validation is deferred to checkpoint3,040.
