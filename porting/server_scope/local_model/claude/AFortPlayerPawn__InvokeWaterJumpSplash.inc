// CONFIDENCE: MEDIUM for control flow (every branch read from 0x142330d30); LOW for Niagara parameter FName constants and the splash default normal.
// Original addresses: exec 0x142a7cf40; native body 0x142330d30. Read order from the thunk: SplashPosition (FVector) first, then SplashScale (float).
// The SplashScale parameter is also pushed as SplashScale * 10 on a second Niagara float parameter.
UCharacterMovementComponent* Movement = CharacterMovement;
UWorld* World = GetWorld();
UNiagaraComponent* JumpSplash = NiagaraPlayerJumpSplashInWater;
if (Movement && World && JumpSplash && JumpSplash->GetNiagaraSystemInstance() && World->GetNetMode() == NM_Client)
{
    if (Movement->NativeRecoveredFloat_0x1d04 <= 0.0f && Movement->NativeRecoveredFloat_0x1d04 != 0.0f)
    {
        if (GLogVerbosityWaterSplash > 4)
        {
            // Logs "Invoked jump water splash with invalid water data %s" with this pawn's name.
            UE_LOG(LogFortWater, Warning, TEXT("Invoked jump water splash with invalid water data %s"), *GetPathNameForLog(this));
        }
    }
    else
    {
        FNiagaraWorldManager* WorldManager = FNiagaraWorldManager::Get(World);
        if (WorldManager)
        {
            if (!ShouldCullWaterSplash(WorldManager, JumpSplash->GetNiagaraSystemInstance(), SplashPosition))
            {
                JumpSplash->SetSplashFlags(true, true);                                              // 0x1441218e0
                const FVector DefaultNormal = SplashDefaultNormal;
                UpdateSplashLocation(JumpSplash, SplashPosition, DefaultNormal, 0, 0, 0);            // 0x144124910 (direct 0x144122ff0 when comp+0xc0 is null)
                JumpSplash->SetRelativeScale3D(FVector(1.0f, 1.0f, 1.0f));                            // 0x144123480 (named SetRelativeScale3D)
                JumpSplash->SetVariableFloat(SplashScaleParamName, SplashScale);                      // vtable 0x870 (LOW)
                JumpSplash->SetVariableFloat(SplashScale10ParamName, SplashScale * 10.0f);            // vtable 0x870 (LOW)
                ApplyWaterSplashParams(Movement->NativeRecoveredWaterSplashData_0x1d04, JumpSplash);  // 0x14234bc90
                JumpSplash->Activate(true);                                                           // vtable 0x268
            }
        }
    }
}
