// CONFIDENCE: MEDIUM - controller lookup and both flag checks read in full; the local-controller predicate (vslot 0x6a8) and the IsUsingFirstPersonFlags helper are named by existing deps; Instigator is cast to APawn where the native reads +0x250 without a class check.
// Original addresses: exec 0x142b4fbe0; body 0x1426d28d0
APawn* InstigatorPawn = Cast<APawn>(Instigator);
AFortPlayerController* PC = InstigatorPawn ? Cast<AFortPlayerController>(InstigatorPawn->Controller) : nullptr;
if (PC == nullptr || !PC->IsLocalPlayerController())
{
    return false;
}
return IsUsingFirstPersonFlags(PC) != 0;
