// Behaviour snippet: UFortGameplayAbility::GetDisplayStats (body only; spliced into the generated member function).
// Original address: 0x1415fa0f0 (asm: MOV RAX,[RCX+0x5d0]; RET). Exec thunk 0x1428f8480.
// Exec thunk: ADD RCX,0x408; CALL [RAX+0x20]. Secondary vptr at +0x408 = 0x145d19ba8 (vtables.json), slot +0x20 = 0x1415fa0f0 (runtime image).
// Adjusted read: obj+0x408+0x5d0 = obj+0x9d8 = UFortGameplayAbility::StatList (reflection offset 0x9d8, class VA 0x142b11120).
// Return type UFortTooltipDisplayStatsList* (same class VA as ReturnValue of UFortTooltipInterface::GetDisplayStats, forward-declared in UFortGameplayAbility.h).
// Declared in generated header: no; reflection declares UFortGameplayAbility::GetDisplayStats (flags 0x54080400, protected).
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this) + 0x408;   // adjusted this (secondary base at +0x408, IFortTooltipInterface)
return *reinterpret_cast<UFortTooltipDisplayStatsList* const*>(Raw + 0x5d0);
