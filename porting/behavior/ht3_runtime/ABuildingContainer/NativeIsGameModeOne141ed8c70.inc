// Original body address: 0x141ed8c70 (helper; tail-jumps to 0x141ed8de0 at 0x141ed8c72)
// Asm: MOV DL,0x1 then JMP 0x141ed8de0. RCX (the context object) passes through unchanged and DL=1 is the GameMode argument of the ported helper UFortGameInstance::NativeIsGameMode141ed8de0.
// Callers include ABuildingContainer::SetSearchLootTierGroup (0x141beb120) and AFortWeaponRanged::GetADSTransitionInTime (0x1426c9f70), which test the bool result.
// CONFIDENCE: HIGH
return UFortGameInstance::NativeIsGameMode141ed8de0(ContextObject, 1);
