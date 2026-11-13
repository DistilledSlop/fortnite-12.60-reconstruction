// CONFIDENCE: HIGH. Recovered original pawn/vehicle virtual family.
// AFortPlayerPawn::GetBaseAimRotation() const override, original 0x1423198d0.
// Complete proposal; promote together with interface/helper/implementer entries.
if (Controller)
    return Super::GetBaseAimRotation();

FRotator Result = GetActorRotation();
constexpr float PackedAngleScale = 0.005493247881531715f; // original bits 0x3bb400b4
const float PackedPitch = static_cast<float>(RemoteViewData32 >> 16) * PackedAngleScale;
const float PackedYaw = static_cast<float>(RemoteViewData32 & 0xffffu) * PackedAngleScale;
// UCOMISS followed by JZ regards either signed zero or unordered as zero.
Result.Pitch = (BlendedReplayViewPitch < 0.0f || BlendedReplayViewPitch > 0.0f)
    ? BlendedReplayViewPitch : PackedPitch;
if (VehicleStateLocal.Vehicle && !NativeVehicleSeatPredicate_14235ca20())
    Result.Yaw = PackedYaw;
return Result;
