// Original body address: 0x1422d4270 (thunk 0x1427a4bc0)
// Static: FortAIPawn is the receiver. A null receiver returns EFortMovementStyle value 1 (MOV EAX,1).
// Otherwise the body calls vtable slot 0x998 (candidate AFortPawn::NativeGetCurrentMovementStyle14231a980, declared non-virtual in the candidate note) with a local out byte and returns the byte it points to.
// CONFIDENCE: MEDIUM
if (FortAIPawn == nullptr)
{
    return static_cast<EFortMovementStyle::Type>(1);
}
uint8 OutValue = 0;
const uint8* StyleValue = FortAIPawn->NativeGetCurrentMovementStyle14231a980(&OutValue);
return static_cast<EFortMovementStyle::Type>(*StyleValue);
