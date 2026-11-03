# Vehicle input acknowledgment timestamp

Original14230e450 is exactly MOVSS [RCX+0x121c],XMM1; RET. Current reflected
RPC declaration takes float Timestamp, matching the original floating argument
register. The Ghidra signature's undefined4 integer argument is misleading.
A neutral non-reflected float member represents the evidenced four-byte storage;
no default, constructor, lifetime or other writer semantics are inferred.

The preceding reflected VehicleInputStateUnreliable starts0x11c0. Its actual
struct size is40 (0x28), ending0x11e8, as established by struct_blocks.json block
1479c7050/constructor142841ce0, matching the original five-qword copy in
ServerUpdateVehicleInputStateUnreliable_Implementation. The next reflected
property SafeZoneAppliedGE starts0x1230. Thus121c..1220 is outside these fields,
not part of the preceding struct. No existing canonical member overlaps it.
This uses the known struct extent rather than guessing that its storage extends
to the next property. Generated field placement is logical source reconstruction,
without asserting original binary class layout. Evidence pins complete native
bytes and the generated C. No build/runtime/tests; source generation required.

Root reviewed and promoted the proposed bodies/fields in generation83: final
2,290 accepted entries,0 rejected. This confirms source generation only; native
build/runtime validation is deferred to checkpoint3,040.
