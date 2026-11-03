// AFortMinigame::StartMinigameCountdown(AFortPlayerState* Starter) -- native 0x1420a7410 (name from log strings).
// SOURCE DRAFT, uncounted, build pending. Timer callback (vtbl 0x14608f968/0x14608f9c0) remains unresolved;
// the direct-start path does not prove its body. Other unresolved helpers/globals are documented below.
#include "Minigame96Bindings.inc"

void AFortMinigame::StartMinigameCountdown(AFortPlayerState* Starter)
{
    if (GetLocalRole() != ROLE_Authority) { return; }
    if (!Starter)
    {
        if (Minigame96_LogEnabled(1)) { Minigame96_Logf(2, TEXT("AFortMinigame::StartMinigameCountdown - Failed. Invalid PlayerState.")); }
        return;
    }
    UWorld* World = GetWorld();                                               // vtbl+0x150
    if (!World)
    {
        if (Minigame96_LogEnabled(1)) { Minigame96_Logf(2, TEXT("AFortMinigame::StartMinigameCountdown - Failed. Invalid world.")); }
        return;
    }

    if (AGameStateBase* GS = World->GameState)                                // World+0x130
    {
        // IsA(AFortGameStateAthena) via class-chain depth compare; GameState+0x1478 int > 0; UFortRuntimeOptions default byte +0xa5
        // (== bCreativeManualBroadcasterStart per bool_bits).
        if (GS->IsA(AFortGameStateAthena::StaticClass())
            && Minigame96_Raw<int32>(GS, 0x1478) > 0
            && UFortRuntimeOptions::GetDefault()->bCreativeManualBroadcasterStart)
        {
            AutoStartDelay = 0;                                               // +0x798 (int32)
            FTimerManager& TM = World->GetTimerManager();
            FTimerHandle& H = Minigame96_CountdownTimer(this);                // +0xb38
            TM.ClearTimer(H);                                                 // native: if (FindTimer 0x144621fb0) InternalClearTimer; then handle = 0
            H.Invalidate();
            Minigame96_Raw<int32>(this, 0x8b0) = 0;                           // float field zeroed (4-byte write)
            return;
        }
    }

    const bool bReject = Minigame94_RejectLateJoin(this);                     // 0x1420997a0
    const bool bOther  = Minigame96_Query142099810(this);                     // 0x142099810
    const int32 Delay0 = AutoStartDelay;
    const bool bDelayOk = !(Delay0 < 1 && Delay0 != -1);
    bool bCanStart;
    if (bDelayOk || bReject || bOther)
    {
        bCanStart = true;
        if (bReject && Delay0 == 0)
        {
            float F = Minigame96_GlobalDefaultCountdown();                    // _DAT_1478c5014
            if (AGameModeBase* GM = Minigame96_WorldGameMode(World))          // 0x141641300
            {
                F = Minigame96_Raw<float>(GM, 0xf88);
            }
            if (F <= Minigame96_GlobalDefaultCountdown()) { F = Minigame96_GlobalDefaultCountdown(); }   // max(F, global)
            AutoStartDelay = (int32)F;
        }
    }
    else
    {
        bCanStart = false;
    }

    if (CurrentState != (EFortMinigameState)0 || !bCanStart || Minigame96_CountdownTimer(this).IsValid() /* raw != 0 */ || !Volume)
    {
        return;
    }
    // Volume gate: byte +0x408 == 1 (unreflected) OR bit0 of +0x2d8 (== AFortVolume::bNeverAllowSaving per bool_bits)
    if (!(Minigame96_Raw<uint8>(Volume, 0x408) == 1 || (Minigame96_Raw<uint8>(Volume, 0x2d8) & 1)))
    {
        return;
    }

    MinigameStarter = Starter;                                                // +0xaa8 (partial write precedes the controller cast)
    AFortPlayerControllerAthena* Ctl = Cast<AFortPlayerControllerAthena>(Starter->GetOwner());   // Owner +0xe0, class-chain check 0x142a3d9f0

    if (!bOther)                                                             // cVar5 captured once, earlier
    {
        if (AutoStartDelay != -1)
        {
            Minigame96_CountdownEndTime(this) = (float)AutoStartDelay + World->GetTimeSeconds();   // +0x8b0 = (float)delay + World+0x540
            TWeakObjectPtr<AFortPlayerControllerAthena> WeakCtl(Ctl);         // 0x142ee3b30
            // native: heap TBaseWeakLambda-style instance (weak(this), captures {this, WeakCtl}); body at vtbl 0x14608f9c0 unresolved.
            FTimerDelegate Delegate = Minigame96_BindCountdownCallback(this, WeakCtl);
            FTimerManager& TM = World->GetTimerManager();
            TM.SetTimer(Minigame96_CountdownTimer(this), Delegate, (float)AutoStartDelay, false);   // rate re-read after the above
            return;
        }
    }
    else
    {
        AutoStartDelay = 0;                                                   // +0x798
        WarmupDuration = 0;                                                   // +0x7a0 (4-byte write)
    }
    Minigame96_CountdownEndTime(this) = World->GetTimeSeconds();              // +0x8b0 = World+0x540
    Minigame96_StartCountdownNow(this, Ctl);                                  // 0x1429bb7f0(this, ctrl)
}
