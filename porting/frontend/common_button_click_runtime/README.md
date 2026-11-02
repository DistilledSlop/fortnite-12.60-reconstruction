# CommonButton base click virtual adapters

The maintained policy restores original native slots +0x4d0 and +0x508 as
`NativeOnClicked` and `NativeGetClickAnalytics`. These source aliases do not
establish binary vtable-layout parity. All future native producers must use the
same `NativeClicked` multicast member.

The original `0x1407d2580` dispatches BP_OnClicked, broadcasts the native multicast,
then broadcasts OnButtonClicked(this) when its list count is nonzero. It calls the
fresh analytics virtual with three real FString outputs. True analytics resolves
UWidget::GetGameInstance and calls the real CommonUISubsystem lookup even though
its result is discarded; the engine collection fallback can populate its cache.
The outputs leave scope in context/type/name reverse order on both branches.
Original `0x1407c1e30` fills GetName (including numeric suffix), None, None and
returns true. This corrects an earlier research label of owning local player:
`0x143d5c530` is GetGameInstance and `0x14078c7b0` is its subsystem accessor.

The generator validates the authentic class, property, BP event and complete
original delegate record, both member views and helper collisions. It checks the
actual in-memory rendered button/delegate/subsystem headers before writing Native
files. Generated files must be regenerated from these templates.

Source replay: /mnt/vibrato/fortnite-work/button-base-click-contract-1/replay.py
and button-click-context-resolver-1/replay.py, using
PYTHONPATH=/tmp/common-input-capstone. Policy refinement and mutation review:
button-base-click-policy-2/. The source image and exact spans are pinned there.

The actual engine diagnostic is BaseClickProbe with -ButtonBaseClickProbe. Its
strict runner case is base-click. It covers callback ordering, mutation/reentry,
weak receiver collection/dynamic compaction, numbered name and real world context
gates. ProcessEvent observation is not a cooked Blueprint override. The discarded
subsystem lookup requires separate debugger observation; a passing transcript alone
does not prove that call or a private cache mutation. FString cleanup follows real
C++ lifetime rules; the probe does not hook per-string buffer deallocation.

This change does not register upstream accepted-click, Initialize, legacy +0x498
routing, or SetStyle. Native callers are not connected to production lifecycle yet.
UE/UHT builds and runtime evidence must be recorded before calling it verified.

Verified baseline: Native d08549a5 / Engine87acab901; client/server builds,
14 actual callback cases, separate real game-instance subsystem observation,
retained diagnostics and both joins pass. Frozen button-base-click-runtime-3 and
its independent receipt audit are linked in HANDOFF.md. Failed probe phases1/2
are preserved; both fixes changed the fixture, not production callback semantics.
Further testing follows AGENTS.md: coherent integration batches and changed risk,
not a full build/regression/archival cycle for each helper or commit.
