# Common action binding data and storage

These templates are a data-building and owned-storage component, not registration,
input routing, a settings provider, or a production adapter. The normal generator
must validate `policy.validate_types(gate)` and copy both templates into CommonUI's
Generated directory before building them. The adapter supplies a resolver whose tag
lookup scans the original InputActions in order for the first exact tag; row lookup
and owner-to-gamepad resolution must follow the source behavior. Resolved data and
selected gamepad index must be valid. The constructor did not provide missing-data
fallbacks or a gamepad bounds check.

The descriptor byte flags, event and input-mode fields deliberately use neutral
names and have no invented defaults. The route pointee is only forward declared;
its identity, behavior and sharing mode remain unproven. The returned record has
owned engine values and weak owner/route references. Map keys are supplied by the
caller; neither key choice nor routing is inferred here. Counter and map are global,
unsynchronized and have no assumed cross-translation-unit lifecycle order.

`test_runtime.py` compiles the templates against a small behavioral shim under
`-ffast-math`; it does not establish UE ownership or ABI parity. Original-image
proofs and replay scripts are in `/mnt/vibrato/fortnite-work/` under
`common-action-binding-record-types-1`, `action-binding-delegate-contract-1`,
`action-binding-registry-storage-1`, and `action-binding-record-constructor-1`.
The reflection contract pins six complete original type records, independently of
those image windows. Actual engine validation and provider integration remain the
caller's responsibility.

The NaN branch is classified by IEEE bits; other thresholds use a floating
comparison so the processor's denormals-are-zero setting affects subnormal
values. Exact floating-point exception behavior is not established. The isolated
`AllocateIdBits` counter step supports wrap testing without resetting global state.
