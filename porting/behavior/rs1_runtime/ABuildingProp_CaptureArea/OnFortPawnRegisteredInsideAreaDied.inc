// Behaviour snippet: ABuildingProp_CaptureArea::OnFortPawnRegisteredInsideAreaDied (body only).
// Original body address: 0x141c130a0 (thunk 0x14274b650), receiver delta 0.
// Evidence: the decompile (39 lines) and the 60-instruction asm contain only read-only work. DamagedActor is tested with IsA against AFortPawn::StaticClass() (callee 0x142a174f0). Its +0x250 controller is tested with IsA against AFortPlayerController::StaticClass() (callee 0x142a3d9f0). The walk over this+0xb98 (count this+0xba0) only compares pointers. Every path returns with no store and no call, so the function has no observable effect.
// CONFIDENCE: HIGH
return;
