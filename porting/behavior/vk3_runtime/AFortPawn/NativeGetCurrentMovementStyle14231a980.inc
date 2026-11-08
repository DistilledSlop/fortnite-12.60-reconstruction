// Helper 0x14231a980: virtual slot 0x998 of AFortPawn, caller AFortPlayerPawn::IsSprinting.
// Asm: MOVZX EAX,byte ptr [RCX+0x53b]; MOV byte ptr [RDX],AL; MOV RAX,RDX; RET
// RCX = this (AFortPawn), RDX = caller-owned uint8 out slot. Caller stores the result from [RDX].
// Offset 0x53b is the reflected member CurrentMovementStyle (Byte).
// CONFIDENCE: HIGH
*OutValue = CurrentMovementStyle;
return OutValue;
