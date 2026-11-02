# Bounded CommonButton constructor defaults

The constructor policy restores seven explicit original defaults:

| Property | Value |
|---|---|
| bApplyAlphaOnDisable | true |
| bSelectable | false |
| bShouldSelectUponReceivingFocus | false |
| bToggleable | false |
| bDisplayInputActionWhenNotInteractable | true |
| bShouldUseFallbackDefaultInputAction | true |
| inherited bIsFocusable | true |

The original constructor `0x140796950` writes byte `+0x2a0` as
`(old & 0xe9) | 0xa1`. Original property setters identify each bit; the two
preserved bits, bInteractableWhenSelected and bHideInputActionWithKeyboard, are
left untouched by this patch. The focus flag comes from the inherited original
Bool setter at `0x143dbe1d0`. The generator guards all six local property records
against the original reflection input before emitting the constructor.

The constructor uses the existing FObjectInitializer parent path and existing
native enabled defaults. FButtonStyle, FSlateSound and reflected delegates retain
their C++ member construction. No memset or placeholder callback is added.
Unreflected original members at `+0x328`/`+0x340`, native selection/delegate
storage, and full class lifecycle remain unreconstructed. These independent
default fixes do not establish complete constructor, selection or menu parity.

`main-button-defaults-original-runtime.json` pins the entire original constructor,
registration/wrapper, six Bool setters and inherited focus property/setter. Run:

```
python3 porting/frontend/confirmation_runtime/verify_original.py /mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin --record porting/frontend/common_panel_runtime/main-button-defaults-original-runtime.json
python3 -m unittest porting/frontend/common_panel_runtime/test_main_button_query.py -q
```

The portable tests compile the exact maintained definitions with deliberately
opposite fixture defaults, verify the seven writes and preserved properties,
check the 160 query-state combinations, and reject altered property records.
The opt-in MainButtonStateProbe's existing defaults row now also requires these
seven values on the CDO and a newly created widget. Its 44-row transcript and
source/binary provenance are required for a UE runtime claim; portable tests
alone are insufficient. Current runtime receipts are recorded in HANDOFF.md.
