// AFortMinigame::DelayedTeleportPlayer(TWeakObjectPtr<AFortPlayerState>) -- native 0x142089b10 (name from log strings).
// SOURCE DRAFT, uncounted, build pending. Also the timer target scheduled by AddMinigamePlayer (delegate method ptr 0x142089b10).
// Frame has a stack cookie (_DAT_1480c4288 ^ rsp) only. Early outs log (verbosity gate noted) and return.
#include "Minigame96Bindings.inc"

void AFortMinigame::DelayedTeleportPlayer(TWeakObjectPtr<AFortPlayerState> WeakPlayer)
{
    if (!WeakPlayer.IsValid())                                                // FWeakObjectPtr_IsValid
    {
        if (Minigame96_LogEnabled(2))
        {
            Minigame96_Logf(3, TEXT("DelayedTeleportPlayer called and PlayerState (%p) has gone invalid"), WeakPlayer.Get());
        }
        return;
    }

    AFortPlayerState* Player = WeakPlayer.Get();
    FMinigamePlayer* Record = Minigame96_FindPlayerRecord(PlayerArray, Player->GetUniqueId());   // 0x14208d7e0(this+0x438, &Player+0x248)
    if (!Record)
    {
        if (Minigame96_LogEnabled(5))
        {
            Minigame96_Logf(6, TEXT("DelayedTeleportPlayer called and player (%s) is no longer in our minigame"),
                            *Minigame96_ObjectLogString(WeakPlayer.Get()));
        }
        return;
    }

    UObject* VolumeLookup = Minigame96_VolumeLookup(Volume);                  // 0x141c74820(this+0x910) -- param_1[0x122]
    if (!VolumeLookup || !Minigame96_VolumeContains(VolumeLookup, WeakPlayer.Get()))   // 0x141c94c20(lookup, player)
    {
        if (Minigame96_LogEnabled(5))
        {
            Minigame96_Logf(6, TEXT("DelayedTeleportPlayer called and player (%s) is no longer in our volume"),
                            *Minigame96_ObjectLogString(WeakPlayer.Get()));
        }
        return;
    }

    if (Minigame96_PlayerNeedsRespawnRaw(WeakPlayer.Get()) == 0)              // byte Player+0xd30
    {
        if (Minigame96_LogEnabled(5))
        {
            Minigame96_Logf(6, TEXT("Player (%s) started joining a minigame and no longer needs to respawn"),
                            *Minigame96_ObjectLogString(WeakPlayer.Get()));
        }
        return;
    }

    Minigame96_PlayerStateSetFlag(WeakPlayer.Get(), true);                    // 0x141a5b590(P, 1)
    if (Minigame96_PlayerStateQuery(WeakPlayer.Get()))                        // 0x141a4b2b0(P) != 0 -> return
    {
        return;
    }
    UWorld* World = GetWorld();                                               // vtbl+0x150
    if (!World)
    {
        return;
    }

    // Native heap-builds a TBaseUObjectMethodDelegateInstance(this, &AFortMinigame::DelayedTeleportPlayer, WeakPlayer) -> FTimerDelegate.
    // Weak ptr value captured here is the ORIGINAL by-value parameter (lStackX_10), re-read after the calls above.
    FTimerDelegate Delegate = FTimerDelegate::CreateUObject(this, &AFortMinigame::DelayedTeleportPlayer, WeakPlayer);
    // Timer manager: World->OwningGameInstance (+0x188) ? GameInstance->TimerManager (+0xc0) : World->TimerManager (+0x320) == GetTimerManager().
    // SetTimer(handle = Record+0x88, delegate, rate 0x3dcccccd == 0.1f, bLoop = false, FirstDelay default). Delegate temp destroyed afterwards.
    World->GetTimerManager().SetTimer(Minigame96_RecordTeleportTimer(*Record), Delegate, 0.1f, false);
}
