// CONFIDENCE: HIGH
// Original 0x14238e8e0; full recovered body. CONFIDENCE: HIGH.
// Requires a reconstructed AFortPlayerPawn::NativeVehicleInputState1194 member
// of type FFortAthenaVehicleInputState and the recovered IsDrivingVehicle body.
if (!MyFortPawn || !MyFortPawn->GetMovementComponent())
{
    return false;
}

const bool bCharacterVehicle = MyFortPawn && MyFortPawn->IsA<AFortCharacterVehicle>();
if (MyFortPawn->IsDrivingVehicle() && !bCharacterVehicle)
{
    // Original COMISS epsilon, ForwardAlpha; SETC also accepts unordered.
    return !(MyFortPawn->NativeVehicleInputState1194.ForwardAlpha <= 0.0001f);
}

UInputComponent* Input = MyFortPawn->InputComponent;
if (!Input)
{
    return false;
}

static const FName MoveForwardName(TEXT("MoveForward"));
static const FName MoveForwardGamepadName(TEXT("MoveForward_Gamepad"));
static const FName MoveRightName(TEXT("MoveRight"));
const bool bForward = Input->GetAxisValue(MoveForwardName) > 0.0001f;
const float ForwardGamepad = Input->GetAxisValue(MoveForwardGamepadName);
const float Right = Input->GetAxisValue(MoveRightName);
const float Magnitude = FMath::Sqrt(ForwardGamepad * ForwardGamepad + Right * Right);
const float Threshold = static_cast<UFortPlayerInput*>(PlayerInput)->GamepadMoveStickDeadZone;
return bForward || (Magnitude > Threshold && ForwardGamepad > 0.0001f);
