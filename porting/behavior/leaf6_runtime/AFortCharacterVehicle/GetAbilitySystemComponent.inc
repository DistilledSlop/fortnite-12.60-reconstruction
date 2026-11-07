// Behaviour snippet: AFortCharacterVehicle::GetAbilitySystemComponent (body only; spliced into the generated member function).
// Original address: 0x141945270 (asm: MOV RAX,[RCX+0x2ca8]; RET). Exec thunk 0x142861520.
// Exec thunk: ADD RCX,0x4c8; CALL [RAX+0x10]. Secondary vptr at +0x4c8 = 0x145df7f98 (vtables.json), slot +0x10 = 0x141945270 (runtime image).
// Adjusted read: obj+0x4c8+0x2ca8 = obj+0x3170 = AFortCharacterVehicle::OverrideAbilitySystemComponent (reflection offset 0x3170).
// Return type UAbilitySystemComponent* per AFortAthenaVehicle.h (virtual GetAbilitySystemComponent() const).
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this) + 0x4c8;   // adjusted this (secondary base at +0x4c8)
return *reinterpret_cast<UAbilitySystemComponent* const*>(Raw + 0x2ca8);
