// CONFIDENCE: HIGH. Original1419c3470; cached weapon and repeated movement query retained.
if (ActionTag.MatchesTag(FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x768)))
{
    AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(GetPlayerPawn());
    if (Pawn)
    {
        AFortWeapon* Weapon = Pawn->CurrentWeapon;
        if (Pawn->GetMovementComponent())
        {
            if (!Pawn->GetMovementComponent()->IsSwimming() &&
                !Pawn->IsInVehicle() &&
                !Pawn->NativeAllowsADSWhileNotOnGround10B0() &&
                !Pawn->bBalloonMovementActivated &&
                !Pawn->IsZiplining() &&
                (!Pawn->IsSurfaceSwimming() || Pawn->bIsWaterJump) &&
                (!Weapon || !Weapon->NativeAllowsADSInAirA80()))
            {
                return false;
            }
        }
    }
}
return AFortPlayerController::NativeCanPerformAction15f0(ActionTag);
