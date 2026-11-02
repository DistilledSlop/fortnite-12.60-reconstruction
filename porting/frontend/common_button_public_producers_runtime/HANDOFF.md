# CommonButton public producers

Unregistered, unbuilt candidate. `declarations.inc` and `definitions.inc` implement
only the three selection entry points, using the exact shared transition routines in
`../common_button_focus_selection_runtime/`. `policy.py` gates CommonButton's full
original function records and exec joins for all five researched producers, bounded
original windows, the +488 vtable target, ownership, existing focus/selection policy,
collisions, and exact rendered selection fragments. Integrate by validating original
and class metadata before writes, applying after focus policy, splicing both fragments,
and validating rendered output. The applied flag is
`common_button_public_selection_producers_runtime`. Compile and test in the coherent
frontend batch. The metadata's protected `SetSelectedInternal` and public callable
functions require UHT confirmation. No generator, native files, build or commit touched.

Original joins: ClearSelection exec 14080a190 jumps 1407ae7e0, which tail-calls
1407f9880(false, false, true). SetIsSelected exec 14080d110 passes the two Blueprint
arguments in source order to 1407f7e90; SetSelectedInternal exec 14080d430 passes
all three in source order to 1407f9880. The existing focus candidate owns those native
bodies and its callback/reentry behavior. The full records are in original-policy.json.

`SetTriggeringInputAction` is the real Blueprint import, but its implementation is
NOT supplied. Exec 14080d8d0 calls 1407fa9e0. That returns if the existing reflected
row +2b0 is non-null; otherwise it calls teardown 140801750 *before* copying the
incoming row, then dispatches virtual +488 freshly from the object, then virtual
+4a0 freshly. The vtable slot +488 is 1407abbc0, and +4a0 is the actual action-widget
update. A simple assignment silently loses observable binding effects and is invalid.

Teardown 140801750 acts only when either row is non-null and the native row +2c0 is
null. If +acc is nonnegative and resolves via 1407bd730, it calls 1408018c0, which
removes the binding from the still-live weak route through 1407f0ab0, then removes
the registry entry and sets the handle to -1. It then resolves game instance,
subsystem and manager, and if the reflected row is valid calls 140801950 on the
manager +c8. The latter walks and removes legacy row-owner registrations; it is not
just CommonActionBindingData::Remove.

Setup 1407abbc0 returns unless reflected row is non-null and native row is null. It
checks existing +acc using 1407bd730. If invalid, it constructs delegates targeting
1407c6960 (accepted click) and 1407a5c70 (fresh +4f8 virtual), a descriptor with the
live row and fields +2d0/+2d1, and obtains the owner player (+290), its router
14078c710, and registers through 1407ed1c0. The returned handle is appended to the
widget's array at +230 and written to +acc. 1407ed1c0 calls the already-maintained
creation entry 140800e60, but also associates the record with a widget/route using
1407be460 or 1407beb40 and 1407a90c0, including a separate fallback, and rejects
when router +98 is set. If the resulting +acc remains invalid setup resolves the
GI subsystem manager and constructs three dynamic delegates from the exact literal
names `HandleTriggeringActionCommited`, `NativeOnActionProgress`, and
`NativeOnActionComplete`, then calls the legacy manager registration 1407ec2d0
with live row and priority. +2a8 priority zero triggers parent-chain priority lookup
before this call. Reentry and ownership in both registration paths remain to recover.
`CommonActionBindingData::Create/Remove` alone cannot replace router and manager
association; no fake binding or row-only fallback should be installed.

The distinct native row setter SetTriggeredInputAction exec 14080d7f0 / receiver
1407fa760 and held panel bindings remain unimplemented; see
`../common_button_click_runtime/source-evidence.md` and
`/mnt/vibrato/fortnite-work/button-triggered-row-producer-1/HANDOFF.md`. No proof
that the public triggering setter calls it was found: +488 reads +2c0 and skips
setup when non-null, rather than invoking the native setter. It still needs its own
full lifecycle for legitimate callers.

Validation: `python3 porting/frontend/common_button_public_producers_runtime/test_policy.py`
PASS on 2026-11-02. Original instruction checks use `os.pread` only; full image was
neither loaded nor hashed. Metadata mutation of ClearSelection exec was rejected.
Original spans include the receiver, teardown, setup, and removal helper. This is
static source evidence and does not establish generated, compiled, or runtime parity.
