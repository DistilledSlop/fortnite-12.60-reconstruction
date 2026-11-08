// Original body 0x14246ceb0 (thunk 0x1429d7fa0).
// Offset evidence: [Player+0x278] = player-state pointer (undeclared, own region of ASOLFortPlayer: member_field NativeRecoveredUint64_0x278).
// GetCurrentPawn (0x1419b6d20) on that state; [pawn+0x130] = RootComponent (reflected); [root+0x1d0..0x1d8] = x,y,z floats (member_fields in USceneComponent).
// Null Player or null root component returns the zero fallback (writable snapshot 0.0 values at 0x148218630/0x148218638).
// CONFIDENCE: MEDIUM
if (Player == nullptr)
{
	return FVector::ZeroVector;
}
static_assert(sizeof(AFortPlayerState) > 0, "AFortPlayerState must be complete");
static_assert(sizeof(AFortPlayerPawn) > 0, "AFortPlayerPawn must be complete");
AFortPlayerState* PlayerState = reinterpret_cast<AFortPlayerState*>(Player->NativeRecoveredUint64_0x278);
AFortPlayerPawn* Pawn = PlayerState->GetCurrentPawn();
USceneComponent* Root = Pawn->RootComponent;
if (Root == nullptr)
{
	return FVector::ZeroVector;
}
return FVector(Root->NativeRecoveredFloat_0x1d0, Root->NativeRecoveredFloat_0x1d4, Root->NativeRecoveredFloat_0x1d8);
