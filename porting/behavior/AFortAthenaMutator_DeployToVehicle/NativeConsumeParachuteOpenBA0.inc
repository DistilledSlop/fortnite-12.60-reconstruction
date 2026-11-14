// CONFIDENCE: HIGH. Original0x141ad6120.
// 0x141ad6120. GetComponentRotation updates the component's world rotation
// conversion cache, even though the returned rotation is discarded.
if (GetLocalRole() == ROLE_Authority && Pawn && Pawn->bLocalIsSkydiving && Pawn->bIsSkydivingFromBus)
{
    if (USceneComponent* Root = Pawn->GetRootComponent())
    {
        (void)Root->GetComponentRotation();
    }
    return 2;
}
return 0;
