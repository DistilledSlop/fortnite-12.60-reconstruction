# Building day-phase callback recovery

Integrated source preserves the public native Blueprint event OnDayPhaseChanged,
exec0x142737bf0, flags0x08020c00, owner0x142733520. Its parameters are
EFortDayPhase::Type CurrentDayPhase, EFortDayPhase::Type PreviousDayPhase, bool
bAtCreation. The generator emits BlueprintNativeEvent and a virtual native
_Implementation. The base implementation at0x141bb0d80 forwards through original
virtual+0x808; NativeOnDayPhaseChanged preserves that additional source dispatch.
Its base target0x1406801c0 is exactly ret0, so the empty base body is recovered
behavior. NativeUsesDayPhaseChange preserves original virtual+0x938, whose base
0x141bc5c50 reads bUsesDayPhaseChange. Nonreflected helper names and const qualifier
are source reconstruction choices. Generated vtable indices do not claim client
binary ABI compatibility. Derived native overrides remain to be recovered.

source_policy.py guards the exact owner, event record, flags and parameter
metadata before attachment. native-evidence.json pins original thunk, bodies,
base virtual table slots and bool setter. This recovers the callback boundary;
the contextual resolver, volume membership lifecycle, registration/immediate
callback and complete SetUsesDayPhaseChange remain open. Promotion62 accepted
all entries without rejection. No build/runtime test after the2040checkpoint;
next testing threshold3040bodies.
