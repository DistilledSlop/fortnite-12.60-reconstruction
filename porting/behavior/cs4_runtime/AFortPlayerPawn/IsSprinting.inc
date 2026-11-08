// Behaviour snippet: AFortPlayerPawn::IsSprinting (body only; spliced into the generated member function).
// Original body: 0x1423320a0 (thunk 0x142a7d4e0).
// Evidence: virtual slot +0x998 = AFortPawn::NativeGetCurrentMovementStyle14231a980 (returns pointer to OutValue); result byte compared to 3 (sprint style).
// CONFIDENCE: HIGH
uint8 StyleOutput = 0;
const uint8* StyleResult = NativeGetCurrentMovementStyle14231a980(&StyleOutput);
return *StyleResult == 3;
