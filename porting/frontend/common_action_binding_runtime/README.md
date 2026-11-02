# Common action binding data and storage

These templates provide owned binding data/storage and original engine-backed
data resolvers. The source-backed creation entry validates and inserts records; router association,
lifecycle registration and input routing remain unfinished. The normal generator
must validate `policy.validate_types(gate)` and copy the data and provider templates into CommonUI's
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

Validated at Native `cf37b21`: client/server builds pass, all 26 actual engine data/storage rows pass, retained diagnostics and both joins pass. Native build1 failure is preserved and fixed through UE4.25 declaration macros and touch-key API. That checkpoint used fixture resolvers. The new provider candidate below is not covered by it. Checkpoint and source release hashes are in the project HANDOFF.


The engine resolver reads the settings CDO InputActions in order, uses
`FDataTableRowHandle::GetRow<FCommonInputActionData>(TEXT(""))`, and resolves a
widget's owning player, its action router, then Common Input through the router's
outer. It reads the unsigned gamepad-type byte live, without clamping. Missing
actions/rows, missing player/router/subsystem, and out-of-range row indexing
remain invalid preconditions. No empty record or index-zero fallback is supplied.
The resolver accesses the existing generated `GamepadInputType` field; this does
not add a reflected getter, input producer, registration or dispatch path.
`original-providers.json` pins 16 bounded original-image spans, rechecked with
`../confirmation_runtime/verify_original.py --record ...`. The two provider
contract documents explain the helper identities and correct the earlier
constructor note that called the router lookup a local-player lookup.

The new opt-in `ActionBindingProviderProbe` uses actual engine settings, typed
DataTable rows (including the original 421-row table), a widget and the live
local-player subsystem collection. Controlled settings and input-byte mutations
are restored; it does not drive original UI or hardware input. At Native `342d083`, both builds, all 20 provider rows, the retained diagnostics
and both joins pass. The checkpoint is `generated/frontend-native-evidence/
action-binding-provider-runtime-1/checkpoint.json` (see HANDOFF for pinned hashes).
This establishes the resolver component; original creation, registration, routing
and interactive UI remain unfinished.


`CommonActionBindingData::Create` recovers the original data creation entry:
name, safe click delegate, tag, row and duplicate validation precede ID allocation
and owned insertion. Tag and row are checked independently. Duplicate identity
is derived name, event and resolved weak owner; other flags/text do not replace
an existing binding. The created record's own ID is the map key, with no wrap
sentinel avoidance. `LogUIActionRouter` is one exported shared category (default
Log, compile All); original Error diagnostics and event labels are retained.
`original-creation.json` pins the full entry, category construction and diagnostic
literals. This does not implement the router's signed-ID acceptance, association,
notification or teardown. At Native `d97565d`, both builds, all 23 CreationProbe cases and exact log diagnostics,
retained diagnostics and both joins pass. The creation checkpoint/release hashes are
in HANDOFF. Build1 failed on a diagnostic include typo; its evidence is preserved
separately from the successful corrected build2. Original router association,
production lifecycle/selection/input dispatch and interactive UI remain unfinished.
