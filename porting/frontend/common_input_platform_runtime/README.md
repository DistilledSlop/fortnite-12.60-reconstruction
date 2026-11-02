# CommonInput platform defaults

Original `FCommonInputPlatformData` constructor `0x14081e5c0` writes scalar
bytes `00 01 00 01 00 01 00` and constructs both arrays empty. The source and
vtable windows, scope, and reproduction are recorded in
[`common_input_soft_load_evidence`](../common_input_soft_load_evidence/README.md).
The reflected enum records identify `1` as `ECommonInputType::Gamepad` and `0`
as `ECommonGamepadType::XboxOneController`. Reflection identifies the arrays
as soft controller classes and controller classes respectively. Bounded raw
property-table reads now independently bind all five Bool setters to bytes
0,2,3,5,6; each setter writes1 to its own byte and returns. The property record,
setter address and instruction-window hashes are in `bool-setters.json`.

`original-types.json` captures the full source reflection records for the struct
and both enums. `validate(gate)` checks their identities, full records and
availability; `apply_struct` independently checks the exact struct and every
emitted member declaration, including each array, before marking the model.
The generated constructor initializes scalar fields in declaration order and
lets each array's C++ default constructor initialize it. It does not write
padding or copy raw object bytes.

Integration: load the policy alongside `common_input_action_row`, call
`validate(gate)` in the promotion seed setup, call `apply_struct(info, model)`
in `TypeGate.build_struct`, and extend the struct renderer with
`declarations(model)` as for the existing action-row policy. Validation and
build of the integrated batch are owned by the caller.
