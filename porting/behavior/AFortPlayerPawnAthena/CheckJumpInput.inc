// CONFIDENCE: HIGH. Complete original 0x141a0e190; build/runtime pending.
auto* Movement = static_cast<UFortMovementComp_CharacterAthena*>(GetCharacterMovement());
const float Remaining = Movement->NativeJumpPenaltyRemaining1e58 - DeltaTime;
Movement->NativeJumpPenaltyRemaining1e58 = Remaining > 0.0f ? Remaining : 0.0f;
AFortPlayerPawn::CheckJumpInput(DeltaTime);
