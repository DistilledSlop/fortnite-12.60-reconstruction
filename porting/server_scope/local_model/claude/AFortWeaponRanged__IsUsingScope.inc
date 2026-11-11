// CONFIDENCE: MEDIUM - bIsTargeting and bUseScopeTargeting are reflected members; FindLocalViewingPlayerController is an existing pawn dep (semantics MEDIUM).
// Original addresses: exec 0x142b4fc10; body 0x1426d2930
AFortPawn* Pawn = Cast<AFortPawn>(Instigator);
if (Pawn == nullptr || !bIsTargeting || !bUseScopeTargeting)
{
    return false;
}
return Pawn->FindLocalViewingPlayerController() != nullptr;
