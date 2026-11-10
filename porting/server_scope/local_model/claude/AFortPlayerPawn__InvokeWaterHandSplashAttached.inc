// CONFIDENCE: MEDIUM for control flow (every branch read from 0x142330b00); LOW for Niagara internals and the splash default vectors.
// Original addresses: exec 0x142a7ce80; native body 0x142330b00. Read order from the thunk: AttachParent (object, reflected index 1) first, then AttachSocketName (FName, index 0).
// Named engine helpers: FNiagaraWorldManager::Get (0x140994050, its log string names it), UWorld::GetNetMode (0x144693810, 3 = client), FNiagaraSystemInstance getter (vtable 0x890).
UCharacterMovementComponent* Movement = CharacterMovement;
UWorld* World = GetWorld();
UNiagaraComponent* HandSplash = NiagaraPlayerHandSplashInWater;
if (Movement && World && HandSplash && HandSplash->GetNiagaraSystemInstance() && World->GetNetMode() == NM_Client && GetCachedWaterSplashCVar() > 1)
{
    if (Movement->NativeRecoveredFloat_0x1d04 <= 0.0f && Movement->NativeRecoveredFloat_0x1d04 != 0.0f)
    {
        if (GLogVerbosityWaterSplash > 4)
        {
            // Logs "Invoked hand water splash with invalid water data %s" with this pawn's name.
            UE_LOG(LogFortWater, Warning, TEXT("Invoked hand water splash with invalid water data %s"), *GetPathNameForLog(this));
        }
    }
    else
    {
        FNiagaraWorldManager* WorldManager = FNiagaraWorldManager::Get(World);
        if (WorldManager)
        {
            // Splash location is the attach parent's cached world position (+0x1d0 x/y, +0x1d8 z); used only for the cull test.
            FVector SplashLocation(AttachParent->NativeRecoveredFloat_0x1d0, AttachParent->NativeRecoveredFloat_0x1d4, AttachParent->NativeRecoveredFloat_0x1d8);
            if (!ShouldCullWaterSplash(WorldManager, HandSplash->GetNiagaraSystemInstance(), SplashLocation))
            {
                // Constant default placement, applied to the splash component.
                const FVector DefaultPosition = SplashDefaultPosition;
                const FVector DefaultNormal = SplashDefaultNormal;
                SetSplashTransform(HandSplash, DefaultPosition, DefaultNormal, false, false, false);   // 0x144122ff0
                HandSplash->AttachTo(AttachParent, AttachSocketName, EAttachLocation::KeepRelativeOffset); // vtable 0x8a0 (LOW)
                ApplyWaterSplashParams(Movement->NativeRecoveredWaterSplashData_0x1d04, HandSplash);       // 0x14234bc90
                HandSplash->Activate(true);                                                                // vtable 0x268
            }
        }
    }
}
