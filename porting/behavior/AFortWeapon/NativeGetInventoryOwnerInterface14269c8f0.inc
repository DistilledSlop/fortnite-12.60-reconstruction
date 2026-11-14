// CONFIDENCE: HIGH
// Original 0x14269c8f0.
if (AFortPlayerController* Controller = Cast<AFortPlayerController>(NativeResolveController1426a07c0()))
{
    return static_cast<IFortInventoryOwnerInterface*>(Controller);
}
if (AFortPawn* Pawn = Cast<AFortPawn>(GetInstigator()))
{
    return Pawn->NativeGetInventoryOwnerInterfaceBD8();
}
return nullptr;
