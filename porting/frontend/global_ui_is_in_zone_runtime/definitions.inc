bool UFortGlobalUIContext::IsInZone() const
{
    const ULocalPlayer* LocalPlayer = Cast<ULocalPlayer>(GetOuter());
    if (LocalPlayer && Cast<AFortPlayerControllerZone>(LocalPlayer->GetPlayerController(GetWorld())))
    {
        return true;
    }

    LocalPlayer = Cast<ULocalPlayer>(GetOuter());
    return LocalPlayer && Cast<AFortPlayerControllerSpectating>(LocalPlayer->GetPlayerController(GetWorld())) != nullptr;
}
