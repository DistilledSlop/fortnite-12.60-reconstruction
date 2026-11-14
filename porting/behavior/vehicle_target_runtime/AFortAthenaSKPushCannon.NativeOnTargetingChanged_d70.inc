// CONFIDENCE: HIGH. Complete original body; native descendant dispatch inventoried.
// 0x1418f3550; NativeVehiclePredicate_0558 preserves original interface virtual dispatch.
if (bTargeting && Pawn && SeatIndex == 0 && NativeVehiclePredicate_0558())
{
    UFortVehicleAimingWeaponComp* Aim = Cast<UFortVehicleAimingWeaponComp>(
        FindComponentByClass(UFortVehicleAimingWeaponComp::StaticClass()));
    if (Aim)
    {
        Aim->StartCameraResetToBarrelTime = GetWorld()->TimeSeconds;
        Aim->bPlayerEnteredThisFrame = true;
    }
}
