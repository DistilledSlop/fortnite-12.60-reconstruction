// CONFIDENCE: HIGH. Complete original body; class lifecycle acceptance remains separate.
// Complete helper1440739b0/1446e25d0; use free helper or public binding.
// CanJumpInternal is protected in stock Character.h, so movement cannot directly call it.
// Signature: bool NativeCanJumpInternalEvent1440739b0(ACharacter* Owner)
struct FParams { bool ReturnValue; } Params{false};
Owner->ProcessEvent(Owner->FindFunctionChecked(FName(TEXT("CanJumpInternal"))), &Params);
return Params.ReturnValue;
