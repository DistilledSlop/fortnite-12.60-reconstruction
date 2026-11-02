# Common action route association: bounded candidate

Unregistered, unbuilt source candidate. Only this directory changed. No generator,
Native, generated Engine files, builds or commits touched. This bounded task ends
at the existing route's binding-array/weak-owner operations; full router creation
and owner registration are not claimed complete.

`CommonActionRouteBindingOperations.h` closes original `1407a90c0` association,
`1407f0ab0` disassociation and `1408018c0` registry teardown using the existing
`CommonActionBindingData::Find/Remove` registry. It creates no new map, route,
provider, row fallback or durable state. It is ready to splice into the real route
once that class is recovered, but must not be enabled as a substitute router.
Required hooks are synchronous original calls, with no defaults:

* Associate/Disassociate notification: freshly call this route's virtual +18.
  Only if true, resolve its weak UObject +28 with `142ee5dd0`, add +a8 to that
  returned owner and call the zero-argument delegate broadcast `140684de0`.
  Original performs no null fallback after weak resolve. Owner type and virtual
  implementation remain unresolved, so these calls must be bound to real recovered
  methods, not a false-return/no-op hook.
* Unregister route removal: invoke the recovered Disassociate body on the actual
  pinned route and its own +18 handle array. Do not remove registry data in the
  hook: the outer operation does so after releasing its strong route reference.

The required inputs are the original route +18 `TArray<int32>`, +08/+10 weak self,
and the actual global record. They are not extra class fields. The forward type
`FCommonActionBindingRoute` is the existing binding candidate's placeholder; no
claim of its original class name or complete constructor/layout is added here.

## Closed semantics and reentry

Association scans IDs, returning immediately for an existing ID without repairing
its weak route or notifying. On a new ID it appends, pins route weak self, replaces
record.Route with that pinned shared pointer converted back to weak, releases the
temporary strong pointer, THEN calls virtual +18 and possibly notifies. An expired
weak self therefore still appends and clears the record route, and may notify.
There is no cached ID/record access after notification.

Removal helper `1407ef330` is stable all-match `TArray<int32>::Remove`, preserving
capacity. If no item removed, disassociation performs neither registry lookup nor
notification. If removed, freshly find global record; if present reset its weak
route WITHOUT checking that it still identifies this route. Notify even when no
registry record exists. No strong ownership is acquired here.

Unregister `1408018c0` looks up every signed value, including negative IDs; sign is
not a gate. If the record's weak route has nonnull raw pointer/control and live
strong count, promote it and remove from that route. Release this strong reference
before registry deletion. Call global Remove using the LIVE handle reference,
then write -1. Both route notification and route destruction can reenter/change
that handle; do not cache it across either. Notification can also remove/reallocate
the registry record, so the source candidate does not access it afterward.

## Acquisition and owner-storage continuation

`1407ed1c0` exact branch remains as in the public-producer handoff: router+98
rejects before Create, created negative IDs or missing global records return -1,
accepted IDs remain successful even without a route/fallback owner. It freshly
looks up the record immediately before `1407a90c0`.

Newly decoded acquisition tree (not implemented): `140805ec0` identifies
CommonActivatableWidget, confirmed in existing
`../common_activatable_panel_native_events.md`. `1407be460(router, out, widget)`
returns empty for null widget. It walks router+50 array of shared root nodes.
Widget+25b selects root-only matching (node weak widget+58) or recursive depth-first
`1407be690`; that recursive helper compares node weak widget+58, then walks shared
children array+70. It returns the first match with strong ownership. `1407beb40`
first attempts the matching CommonActivatableWidget itself via that function; if
none and either not activatable or +25b false, it asks parent discovery
`1407be8a0(widget)` and searches that result. Do not replace with one arbitrary root.

Fallback `143d5a140(widget)` is tested only when primary route null; its temporary
shared reference is RELEASED before testing the saved raw pointer. On nonnull,
`1407c4240(router, widget)` returns/creates owner record and adds ID uniquely to
record+8 array, without assigning record.Route or notifying here. c4240 uses
router+30 array, calls `14078b810` to find by widget, else appends 0x30-byte record:
weak widget+0, IDs array+8, byte+18=0, array of 0x18-byte shared-pair/int entries+20.
Full payload identity and `14078b810` equality remain to recover. The inverse
router-level helper `1407f0b40` first tries route pin/removal; if not, resolves
binding owner and scans router+30 weak-owner records then removes ID from +8.
This owner fallback is distinct from `1408018c0` and is not supplied as source.

Next bounded tasks: recover real route constructor/vtable (+18 notification
predicate and weak-owner+28 identity), then wire the three operation bodies;
separately finish node acquisition/owner-record creation and fallback removal.
Router registration `1407ed1c0` must not be promoted before those real hooks and
storage are available. The separate legacy manager registration remains outside
this branch.

## Validation and exact commands

2026-11-02 PASS:
`python3 porting/frontend/common_action_route_registration_runtime/test_runtime.py`

This validates nine bounded original windows (original-policy.json) with os.pread,
then compiles/runs the candidate against a minimal C++ harness using g++ C++17,
-Wall -Wextra -Werror and AddressSanitizer/UndefinedBehaviorSanitizer. Checks cover
duplicate no-repair/no-notify, expired weak-self replacement, stable duplicate
removal/capacity, missing-record notification, registry erasure during notification,
strong lifetime through route callback and destruction-before-registry-removal with
live handle mutation, negative/absent handle teardown. These focused ownership/
reentry checks are not a UE build or real frontend/runtime parity proof.

Original decoder command, always bounded, no full-image reads/hashes:
`PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_action_route_registration_runtime/disassemble.py START END`

Windows decoded: a90c0..a91e0, f0ab0..f0c60, ed1c0..ed3f0,
be460..be750, be690..be8a0, beb40..bee30, ef330..ef460,
8018c0..801946, c4240..c4470 (all prefixed 0x1407 except 0x1408018c0).
Input was `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`.
No recovered image bytes are stored here.

Primary remains planner/orchestrator/reviewer; bounded delegation Luna medium for
small reads, Terra medium/high regular, Sol medium complex, fork_turns none.
Preserve economical input/output reads and rotate at bounded endpoints.
