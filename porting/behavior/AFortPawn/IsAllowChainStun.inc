// Behaviour snippet: AFortPawn::IsAllowChainStun (body only; spliced into the generated member function).
// Original address: 0x142a1cc50
// Decompile source: porting/generated/decomp/AFortPawn/IsAllowChainStun.c
// Proxy score: 92.2 (portability.json)
// Inferred: Decompile compares the ControlRecoveryBehavior byte (+0x53c) with 1. Enum value 1 is LimitedControl in EFortControlRecoveryBehavior.h. Enum width of 1 byte assumed.
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: MEDIUM
return ControlRecoveryBehavior != EFortControlRecoveryBehavior::LimitedControl;
