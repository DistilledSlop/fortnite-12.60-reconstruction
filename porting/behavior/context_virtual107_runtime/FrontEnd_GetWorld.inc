// CONFIDENCE: HIGH
// Constructor-proven slot 0x150: 0x144cd37c0, fallback 0x141baa780.
const ULocalPlayer* Player = Cast<ULocalPlayer>(GetOuter());
if (Player)
{
    if (UWorld* World = Player->GetWorld())
    {
        return World;
    }
}
return Super::GetWorld();
