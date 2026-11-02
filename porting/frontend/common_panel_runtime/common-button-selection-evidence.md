# CommonButton selection decision core

`CommonButtonSelectionCore.h` implements the recovered public setter, internal
transition, unconditional clear, style refresh, and base selected/deselected notification order.
The portable core requires operations supplied by a future UObject adapter. It
does not register reflected methods, invent state defaults, or replace unresolved
lifecycle, style, delegate, input-action or feedback operations with empty bodies.

The original public setter at `0x1407f7e90` samples hover before and after its
selectability/change/toggle gates. Selection forwards the feedback argument;
deselection forces it true. The internal transition at `0x1407f9880` writes state,
refreshes style, and branches on freshly read selection. Selected notifications
precede the toggle test and interaction change; deselected notifications follow
the interaction change. Both paths refresh the input action. Interaction flags
are read after weak internal-button resolution. Clear at `0x1407ae7e0` bypasses
the public gates. Style refresh at `0x1407f5ce0` resolves the weak internal
button, returns immediately if unavailable, then chooses selected, normal, or
disabled style from actual state before updating the button and emitting the
text-style event. Base notification bodies at `0x1407d39f0`/`0x1407d2b10` always
emit the Blueprint event and text-style event, with the notification parameter
gating only the intervening singlecast and multicast.

The companion JSON pins all six bodies, including the internal transition's
tail jump to `0x140803050`, and six original reflected signatures. State accessors
in the eventual adapter must read the real native fields; explicit callbacks
may reenter, so their effects must remain visible to subsequent reads. The
style/event and weak lifecycle proofs are separately retained under
`/mnt/vibrato/fortnite-work/common-button-{style-refresh-contract,internal-binding,initialize-closure}-1`.

Validation:

```
python3 -m unittest porting/frontend/common_panel_runtime/test_button_selection_core.py -q
python3 porting/frontend/confirmation_runtime/verify_original.py /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin --record porting/frontend/common_panel_runtime/common-button-selection-evidence.json
```

The compiled tests exercise public gates, repeated selection, forced feedback on
deselection, unconditional clear, notification gating, and state changes during
style, selected-event and weak-resolution callbacks. They verify this core's
operation order; they do not establish UObject, Slate, original pixel, or full
CommonButton parity. Runtime registration awaits the actual adapter and reachable
callback/lifecycle dependencies, followed by UE validation. No progress credit
or milestone completion follows from these portable tests alone.
