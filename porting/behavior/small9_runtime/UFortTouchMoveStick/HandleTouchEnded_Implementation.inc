// Behaviour snippet: UFortTouchMoveStick::HandleTouchEnded_Implementation (body only; spliced into the generated function).
// Original body address: 0x144dc5280. Offset evidence: MOV byte [RCX+0x488],0. +0x488 is below UFortTouchMoveStick lowest own offset 0x490, so it is inherited parent storage; raw byte store used (asm writes the whole byte, not one bit). bIsTouched bit member shares the byte.
// CONFIDENCE: MEDIUM
uint8* Raw = reinterpret_cast<uint8*>(this);
*reinterpret_cast<uint8*>(Raw + 0x488) = 0;
