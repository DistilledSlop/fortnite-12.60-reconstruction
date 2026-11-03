# FName field-copy setter proposals

`python3 porting/behavior/bulk_copy_setter_runtime/prepare.py` checks the pinned
runtime image SHA-256 and scans reflected native functions. It emits isolated
`.inc` bodies and `/tmp/gameplay-bulk-copy-setter/{manifest-entries,report}.json`.
Existing bodies must match exactly. These are source proposals, pending promotion
and build/runtime validation.

The accepted family has one by-value FName parameter with exact property flags,
a selected unique non-interface FortniteGame/FortniteUI class, and a complete
exec thunk matching both FFrame extraction paths and P_FINISH. Both branch targets
are verified. The only native call must be a complete leaf consisting solely of
`mov qword ptr [rcx+offset], rdx; ret`. The offset must uniquely map to a scalar
reflected FName in the class ancestry, accessible in the generated header, and
both parameter and field resolve to FName through the generator. RPCs, events,
static functions, reference/out parameters, callbacks, and other leaf effects are
rejected. No inferred storage or nontrivial struct lifetime is involved.
