# CommonButton interaction-state producers

The original CommonButton constructor **140796950** initializes native state
bits **02 and04** at byte+2a1 to true. The interaction getter **1407cf620**
requires both bits, UWidget::GetIsEnabled, and visibility other than ordinal1/2
(Collapsed/Hidden). These are native state fields, distinct from the reflected
button flags at+2a0. Their original C++ member names are not recovered.

Bit02 is controlled by the actual **SetIsEnabled(bool) override1407f78f0**.
CommonButton's installed vtable1459f0940 selects it at slot268. The original
UWidget class registers49 native functions; its SetIsEnabled name binds
exec143dc51d0, which dispatches through that same slot. CommonButton's original
class getter names `/Script/CommonUI.CommonButton`, passes ctor thunk140808120,
and that thunk jumps to140796950. This ties the defaults and setter to actual
typed class/API identities, rather than inferring them from nearby code.

The setter preserves meaningful ordering and transition guards:

- Enabling first calls the original UWidget setter143d8bb50, then returns if
  native bit02 is already set. Otherwise it sets that bit, updates the internal
  button and its custom Slate widget, calls visual helper1407f5ce0 and invokes
  the enabled hook through slot4e0. That hook calls BP_OnEnabled and then
  OnCurrentTextStyleChanged.
- Disabling first clears the underlying Widget enabled flag, then returns if
  native bit02 is already clear. Otherwise it clears the bit, updates internal
  button/Slate state, calls the visual helper and invokes BP_OnDisabled via4e8.
- After either transition it notifies the optional private receiver at+b68,
  passing the new enabled value through that receiver's slot268.

Bit04 has the separate interaction setter1407f7a20, already recovered by the
input dependency worker. It has additional internal Slate and receiver effects.
The complete constructor, both setters and visual/internal-button/callback
dependencies still require source integration. Two booleans alone do not port
these methods, and no runtime or product credit follows from this proof.

The completed late image and proof are pinned by the standard-library replay:

```sh
python3 -I porting/frontend/inspect_common_button_state.py \
  --image /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin \
  --proof /tmp/common-button-native-state-root-1/proof.json \
  --output /tmp/fresh-common-button-state-replay.json
```

The proof SHA-256 is
`ef1267eb08186127fc3e868d88647740fb881e605df753f8ad831c00c82b36b9`.
It preserves77 original windows, including the full537-byte constructor and
230-byte enabled setter. Replay verifies bytes and the override binding;
semantic classification additionally comes from the reviewed registration,
constructor and setter dataflow. No original process, shared Ghidra project,
Native source, Engine source or stage was modified.

Additional hook binding (2026-11-01): installed slot4e0 targets1407d2bd0;
wrapper140808d10 uses FName1480d22c8, initialized from UTF-16 BP_OnEnabled
at145a02120 by1401c5e40. Slot4e8 targets1407d2b70; wrapper140808c90 uses
FName1480d22b8, initialized from BP_OnDisabled at145a02098 by1401c5e00.
Both hooks then dispatch slot4f0 (OnCurrentTextStyleChanged) unconditionally.
The visual helper already sends that notification when the inner resolves,
so an ordinary enabled transition has two text-style notifications, separated
by the BP enabled/disabled event. Bounded replay and exact windows:
`/tmp/main-button-init-callbacks-1/replay-enabled-hooks.py` (PASS).

The complete IsInteractionEnabled query is now emitted through the guarded
main-button-query generator policy, with its real BlueprintPure const native
signature and the two native state defaults established by the constructor.
This query work does not implement the remaining constructor fields, state
producers or Initialize/callback graph. NativeIsInteractionEnabled is a C++
forwarder for the global input adapter, not an extra reflected function.
