// CONFIDENCE: HIGH
// Original 0x1423909c0: retain both virtual GetWorld calls.
if (GetLocalRole() == ROLE_Authority)
{
    return false;
}
if (GetWorld() && GetWorld()->bIsTearingDown)
{
    return false;
}
ULocalPlayer* LocalPlayer = GetLocalPlayer();
UCommonInputSubsystem* Input = LocalPlayer ? LocalPlayer->GetSubsystem<UCommonInputSubsystem>() : nullptr;
return Input && Input->CurrentInputType == ECommonInputType::Touch;
