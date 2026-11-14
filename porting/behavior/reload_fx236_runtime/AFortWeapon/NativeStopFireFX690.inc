// CONFIDENCE: HIGH
// 0x1426b60f0, virtual +0x690. The timer delegate calls this virtual again.
if (GetLocalRole() == ROLE_Authority)
    return;
UWorld* World = GetWorld();
if (!bIsPlayingFireFX)
    return;
if (bFireFXTriggered)
{
    const float Minimum = MinimumTimeForPersistentFX;
    if (Minimum > 0.0f && World)
    {
        const float Elapsed = World->GetTimeSeconds() - PersistentFXStartTime;
        // COMISS elapsed,minimum; JNC cleanup: unordered elapsed schedules as well.
        if (!(Elapsed >= Minimum))
        {
            FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &AFortWeapon::NativeStopFireFX690);
            World->GetTimerManager().SetTimer(NativePersistentFXTimer3D0, Delegate, Minimum - Elapsed, false, -1.0f);
            return;
        }
    }
}
for (int32 Index = 0; Index < 3; ++Index)
{
    USoundBase* Sound = nullptr;
    if (Index == 0)
    {
        APawn* Pawn = GetInstigator();
        AFortPlayerController* PC = Pawn ? Cast<AFortPlayerController>(Pawn->GetController()) : nullptr;
        if (PC && PC->IsLocalController() && PrimaryFireStopSound1P)
            Sound = PrimaryFireStopSound1P;
    }
    if (!Sound)
        Sound = PrimaryFireStopSound[Index];
    if (Sound)
    {
        const FVector Location = GetRootComponent() ? GetRootComponent()->GetRelativeLocation() : FVector::ZeroVector;
        FortWeaponNative::PlayStopSound(GetWorld(), Sound, Location);
    }
}
for (UAudioComponent* Audio : FireAudioChannels)
    if (Audio)
        Audio->FadeOut(PrimaryFireSoundFadeOutTime, 0.0f, EAudioFaderCurve::Linear);
if (World)
    World->GetTimerManager().ClearTimer(NativeFireFXTimerB18);
NativeFireFXTimerB18.Invalidate();
bIsPlayingFireFX = false;
if (bFireFXTriggered)
{
    bFireFXTriggered = false;
    OnStopWeaponFireFX();
}
