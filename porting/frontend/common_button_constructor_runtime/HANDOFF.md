# CommonButton constructor and immediate member destruction review

Source review only; root owns generator/native/build. No duplicate constructor is
needed. The maintained constructor and native enabled fields are emitted by
`common_panel_runtime/main-button-query-{declarations,definitions}.inc` and its
existing `policy.py`; the original constructor window is already hash-pinned in
`common_button_initialize_runtime/original-policy.json`. Exact original function
is `140796950:140796b69`, including the `ret` at `140796b68`.

The original calls its base constructor first, then writes +2a0 as
`(old & 0xe9) | 0xa1`. Original reflection setter records and independently
bounded setter disassembly establish bits: alpha 01, selectable 02, select-on-focus
04, interactable-when-selected 08, toggleable 10, display-input 20,
hide-keyboard 40, fallback 80. Existing emitted assignments exactly set 01/20/80
and clear 02/04/10, preserving 08 and 40. Do not assign the preserved bits.
It also clears native selected +2a1 bit 01, sets enabled/interaction bits 02/04,
and sets inherited UUserWidget +1b4 bit 02 (focusable). Existing source supplies
these through the selected and enabled member initializers and bIsFocusable ctor
assignment. Base construction goes through `Super(ObjectInitializer)`; no
post-initialization reset is appropriate.

Original reflection identifies ClickMethod/TouchMethod/PressMethod at +2a2/3/4;
the constructor does not write these bytes. Initialize sibling aliases initialize
them to enum zero, which is their UObject zero-allocation value. Original writes
several zero states in native member constructors and byte +2d0 zero; common
reflected plain fields receive UObject zero allocation, while the row handles,
sounds, dynamic delegates, button styles, and native delegates use C++ member
construction. The normal target `FSlateSound` default clears its resource and
`FButtonStyle` constructs its brushes; style member constructor comparison was
not exhausted, so byte-level style constructor parity is not claimed. Three
style constructor calls in original are at +360/+5d8/+850. The original weak
internal pointer at +ad0 is index -1 and serial 0; target `TWeakObjectPtr`
default Reset provides this null state. The separate +acc dword is -1; its
identity/use remains unresolved and must not be equated with that weak pointer.
The six native multicast members at +ad8/+af0/+b08/+b20/+b38/+b50 are empty
with allocation/count zero and internal max/lock value 2; candidate policies
supply six actual UE native multicast delegates (clicked, double clicked,
hovered, unhovered, focus, selected) with normal C++ lifetime. No raw-layout
claim is made for UE version differences or omitted unrelated native members.

Original virtual deleting destructor `1407a7a50:1407a7a84` calls
`14079ba20:14079bc42`. The latter destroys those six native delegate slots in
reverse order, then each of the three styles in reverse order and their four
brush subobjects, the two SlateSound shared payloads, the five dynamic multicast
payloads, the two earlier member allocations, then tail-dispatches base
destruction. C++ member destruction and UObject base lifecycle should own those
resources; do not add a manual destructor or duplicate frees. Some original
native members remain unidentified, so this review is not evidence of full
CommonButton lifecycle parity outside the currently represented graph.

Bounded verification (no whole-image read):

```
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_button_initialize_runtime/disassemble.py 0x140796950 0x140796b69
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_button_initialize_runtime/disassemble.py 0x1407a7a50 0x1407a7a84
PYTHONPATH=/tmp/common-input-capstone python3 porting/frontend/common_button_initialize_runtime/disassemble.py 0x14079ba20 0x14079bc42
```

The reflected setter addresses were loaded from the original JSON for block
`0x145a01210`, with each individual eight-byte image window read via the bounded
`disassemble.py` reader. The constructor's existing reflection guard covers the
six altered flags; the full constructor source hash guard belongs to Initialize's
policy and must actually be invoked by integration. This is review evidence,
not a generation, build, runtime, or full native-layout parity result.
