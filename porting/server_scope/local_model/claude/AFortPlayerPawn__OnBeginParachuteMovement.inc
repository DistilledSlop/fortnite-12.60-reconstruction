// CONFIDENCE: LOW. Original addresses: exec 0x142a7d7f0 dispatches via vtable slot 0xe50 to native body 0x142335930 (228 lines, read in full). Helpers read to depth two. LOW items: glider/FX internals, virtual slots 0x1208, 0x1330, 0x520, 0x530, 0x538, 0x6b0, 0x2260, and the omitted 0x140c31764 -> 0x1495a4a24 dispatch, which is not a no-op.
// Shorthand: Role is pawn+0xf0; CharacterMovement is pawn+0x280; pawn+0x10e7 holds the suppress bits, pawn+0x10e8 the parachute open/forced bits.
bParachuteMovementActive = true;                       // pawn+0x10e6 := 1
NativeRecoveredInt_0x3f8 = 1;                          // pawn+0x3f8 := 1
if (!(NativeRecoveredByte_0x10e7 & 1))
{
    if (Role == ROLE_Authority)
    {
        // MISSING: 0x140c31764 -> 0x1495a4a24 calls the skydiving virtual dispatch; the previous no-op assumption was incorrect.
        UWorld* World = GetWorld();
        // 0x140f95ae0: timer manager is World+0x188 ->+0xc0 when that owner exists, else World+0x800 (read in full).
        FTimerManager& TimerManager = GetWorldTimerManager(World);
        // Deferred callback 0x142317c40: calls virtual slot 0x1330 on this with argument 0 (LOW name).
        FTimerDelegate StartDelegate = [this]() { NativeVirtual_0x1330(0); };
        FTimerHandle StartHandle;
        TimerManager.SetTimer(StartHandle, StartDelegate, 0.1f, false);   // 0x1446278d0 (FTimerManager::SetTimer, read in full)
    }
    // 0x14234d3e0 (read in full): sets the open flags; the CVar at 0x1478de4b4 > 0 selects open-by-default (LOW name).
    SetParachuteOpenFlag(GParachuteOpenByDefaultCVar > 0);
    if ((NativeRecoveredByte_0x10e8 & 0x20) && Role == ROLE_Authority)
    {
        UObject* ParachuteProvider = NativeRecoveredPtr_0x4c8;
        UObject* ParachuteComponent = ParachuteProvider ? ParachuteProvider->GetParachuteComponent() : nullptr;   // vtable 0x10 (LOW)
        if (ParachuteComponent && ParachuteComponent->FindOwnedGliderPackSet(UFortGliderPackSet::StaticClass())) // 0x1406c7d70 owner search (read in full)
        {
            // Glider charges: if the condition object at +0x120 does not pass against FortGlobals+0x7e0, reset CurrentCharges to -1.
            if (!ParachuteComponent->GliderCondition->CheckCondition(GetFortGlobals()->NativeRecoveredField_0x7e0))   // vtable 0x18 (LOW)
            {
                // 0x1406a8e40 (read in full) looks CurrentCharges up once and logs "Failed to find %s %s in %s" when it is missing.
                static FProperty* CurrentChargesProperty = FindPropertyChecked(UFortGliderPackSet::StaticClass(), TEXT("CurrentCharges"));
                ParachuteComponent->UpdateGliderPackCharges(CurrentChargesProperty, -1.0f);   // 0x1406ebb90 + 0x1406b8620 (marks dirty; LOW)
            }
        }
    }
}
UpdateParachuteStateFromMovement();                    // 0x14235bbf0 (read in full)
ParachuteVirtual_0x1208();                             // vtable 0x1208 on this (LOW name)
CacheParachuteFallSpeedFromPawn(CharacterMovement);    // 0x1420edbc0 (read in full): movement+0x1d00 = owner+0x1d8 when present, else FLT_MAX
if (NativeRecoveredPtr_0x2490 == nullptr)
{
    if (!(NativeRecoveredByte_0x320 & 8))
    {
        SetParachuteCosmeticsAsync(true);              // 0x142332fe0 (read in full: applies a loaded cosmetic or queues an async load)
    }
}
else
{
    SetParachuteCosmetics(true);                       // 0x141a1c3a0 (read in full)
}
ClearParachuteTimer();                                 // 0x142353760 (read in full): clears the timer at this+0x449 if set
if (NativeRecoveredPtr_0x4bd)
{
    NativeRecoveredPtr_0x4bd->VirtualRelease_0x538();  // vtable 0x538 (LOW)
    NativeRecoveredPtr_0x4bd = nullptr;
}
// PlayerState comes from this+0x238 (param_1[0x47]). Zone bit: PlayerState+0xd00 bit 2, only when it is a FortPlayerStateZone.
AFortPlayerStateAthena* PlayerState = PlayerStateOf(this);
bool bZoneFlagSet = false;
if (PlayerState && PlayerState->IsA(AFortPlayerStateZone::StaticClass()))   // 0x142a92a00 (named)
{
    bZoneFlagSet = (PlayerState->NativeRecoveredByte_0xd00 >> 2) & 1;
}
// The spawn result is stored at this+0x4bd only on the paths that spawn (the original skips the store otherwise).
UObject* SpawnedParachuteComponent = nullptr;
bool bSpawned = false;
if (!IsParachuteAllowed())                              // vtable 0x6b0 (LOW)
{
    const bool bSkip = ((NativeRecoveredByte_0x10e7 & 4) && !bZoneFlagSet)
        || (ParachuteSkipOwnerLookup(this) != nullptr)  // 0x142321a70 (read in full: finds a matching weak owner)
        || (PlayerState == nullptr);
    if (!bSkip)
    {
        if (PlayerState->IsA(AFortPlayerStateAthena::StaticClass()) && !IsPlayerStateDisabledForParachute(PlayerState)) // AFortPlayerStateAthena_GetPrivateStaticClass; 0x14241bb20 (read in full)
        {
            // UGameplayStatics::SpawnSoundAttached (0x144230ed0, read in full).
            SpawnedParachuteComponent = SpawnSoundAttachedForParachute(NativeRecoveredPtr_0x4c8_Sound, NativeRecoveredPtr_0x4f, 0);
            bSpawned = true;
        }
    }
}
else
{
    // Attached FX: 0x144230c30 -> 0x14420cbe0 spawn, then virtual 0x530 sets its parameters (LOW).
    SpawnedParachuteComponent = SpawnParachuteFXAttached(GetWorld(), NativeRecoveredPtr_0x4c3, 1.0f, 1.0f);
    bSpawned = true;
}
if (bSpawned)
{
    NativeRecoveredPtr_0x4bd = SpawnedParachuteComponent;
}
if (NativeRecoveredPtr_0x4bd)
{
    NativeRecoveredPtr_0x4bd->VirtualSetVolume_0x520(NativeRecoveredInt_0x4be, 1.0f, 0);   // vtable 0x520 (LOW)
    StartParachuteCheckTimer(this);                    // 0x1423526c0 (read in full): arms a 0.75 s timer on this+0x449 that runs 0x14230fef0 (read in full)
}
AFortPlayerControllerZone* ZoneController = NativeRecoveredPtr_0x4a;   // this+0x4a
if (ZoneController && ZoneController->IsA(AFortPlayerControllerZone::StaticClass()))   // 0x142a6ee20 (named)
{
    ZoneController->VirtualNotify_0x2260();            // vtable 0x2260 (LOW)
}
