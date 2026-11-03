// CONFIDENCE: HIGH
// Native 0x144cb7c20: cast the outer to ULocalPlayer, resolve its controller in
// this context's world, and optionally require an AFortPlayerController.
ULocalPlayer* Player = Cast<ULocalPlayer>(GetOuter());
if (!Player)
{
    return nullptr;
}
APlayerController* Controller = Player->GetPlayerController(GetWorld());
return bAllowNonFortController ? Controller : Cast<AFortPlayerController>(Controller);
