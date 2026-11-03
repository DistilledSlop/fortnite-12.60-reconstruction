// AFortMinigame::AssignPlayerToAvailableBucket(AFortPlayerState*) -> int32 -- native 0x142083810 (name from its log strings).
// SOURCE DRAFT, uncounted, build pending. Helpers: Minigame96Bindings.inc. Preserved: branch order, string mode tags, float math,
// no zero-divisor guards (native IDIVs fault on Num==0 / TeamCount==0), partial bucket append before spectate decision.
#include "Minigame96Bindings.inc"

int32 AFortMinigame::AssignPlayerToAvailableBucket(AFortPlayerState* Player)
{
    int32 Index = NumTeams;                                  // iVar11 = *(int*)(this+0xaa0)
    FString Mode;                                            // pwStack_48 / uStack_40 (heap TCHAR buffer)
    const bool bCustom = Minigame96_Query1420a6bc0(this);    // cVar2 = 0x1420a6bc0()

    bool bOverride = false;                                  // cVar3
    if (Minigame96_GlobalFlag1478c5044() == 1)               // _DAT_1478c5044 == 1  (declared below)
    {
        if (Minigame94_RejectLateJoin(this))                 // 0x1420997a0
        {
            // (this+0x228) address is never null; obj = vtbl+0x10(sub-object)
            if (UObject* Sub = Minigame96_SubObject228Resolve(this))
            {
                Minigame96_Prep1429da540();                  // 0x1429da540() between resolve and vtbl+0x610
                UObject* R = Minigame96_ResolveVtbl610(Sub);
                if (R && Minigame96_Raw<void*>(R, 0xb0) != nullptr)
                {
                    bOverride = Minigame96_Query142700740(); // 0x142700740() ; result replaces cVar3
                }
            }
        }
    }

    if (Index == -1 || bCustom || bOverride)
    {
        // "Custom Teams": team = (uint8)(PlayerTeamValue - 3) % PlayerBuckets.Num()   (64-bit remainder of unsigned byte by int; no zero guard)
        const uint8 TeamValue = Minigame94_PlayerTeamValue(Player);          // Player+0x318 vtbl+0x28
        Index = (int32)((int64)(uint8)(TeamValue - 3) % (int64)PlayerBuckets.Num());
        Mode = TEXT("Custom Teams");
    }
    else
    {
        if (bTeamMinigame)                                                   // byte +0x91a (bool_bits)
        {
            Index = -1;
            float Best = 1.0f;
            for (int32 I = 0; I < PlayerBuckets.Num(); ++I)
            {
                const int32 Keep = (Index != -1) ? Index : I;                // iVar4
                const float Current = (float)PlayerBuckets[I].PlayerIds.Num()
                                    * (1.0f / (float)(Minigame96_PlayerArrayNum(this) + 1));   // pfVar10[3] (+0x10) * (1/(Players.Num()+1))
                Index = Keep;
                if (Current < PlayerBuckets[I].DesiredTeamSizePercent && Current < Best)
                {
                    Index = I;
                    Best = Current;
                }
            }
            if (Minigame96_LogEnabled(5))
            {
                Minigame96_Logf(6, TEXT("AFortMinigame::GetBestAvailableTeamBucket - Returning bucket %d with current percent %f of total players."),
                                Index, (double)Best);                        // args dropped by decompiler: inferred (Index, Best)
            }
            Mode = TEXT("Random Teams");
        }
        else
        {
            // first empty bucket
            const int32 Num = PlayerBuckets.Num();
            int32 Found = 0;
            bool bHaveEmpty = false;
            for (int32 I = 0; I < Num; ++I)
            {
                if (PlayerBuckets[I].PlayerIds.Num() < 1) { bHaveEmpty = (Found < Num); break; }
                ++Found;
            }
            if (!bHaveEmpty)
            {
                Index = Minigame96_AddBucket(this, true);                     // 0x1420830a0(this,1)
                if (Minigame96_LogEnabled(4))
                {
                    Minigame96_Logf(5, TEXT("AFortMinigame::AssignPlayerToAvailableBucket - Added new bucket for %d for player %s"),
                                    Index, *Minigame96_PlayerIdString(Player)); // args inferred (bucket, id string)
                }
            }
            else
            {
                Index = Found;
            }
            Mode = TEXT("Separate Teams");
        }
    }

    if (Index == -1)
    {
        // Fallback: append a new bucket. NOTE native reuses the register: after this block Index holds the OLD PlayerBuckets.Num().
        int32 TeamCount = 0;                                                  // iVar8
        for (const FMinigameTeam& T : TeamArray) { if (T.bHasBucketAvailable) { ++TeamCount; } }   // byte +0x24 of 0x28 stride
        const int32 OldNum = PlayerBuckets.Num();                             // iVar11 (also the new bucket's index)
        // native sets Num first, then grows storage if Max < Num; element is zero-initialized below
        PlayerBuckets.AddUninitialized(1);
        FMinigamePlayerBucket& B = PlayerBuckets[OldNum];
        FMemory::Memzero(&B, 4);                                              // +0..3 TeamIdAtGameStart/TeamIdAtRoundStart/pad = 0
        FMemory::Memzero(reinterpret_cast<uint8*>(&B) + 8, 16);               // +8..0x17 PlayerIds data/num/max = 0 (no ctor)
        B.DesiredTeamSizePercent = -1.0f;                                     // 0xbf800000
        B.TeamIdAtRoundStart = TeamArray[(int32)((int64)OldNum % (int64)TeamCount)].TeamIndex;   // IDIV, no guard
        Mode = TEXT("Fallback");                                              // native pointer-compare vs literal is always true
        Index = OldNum;
        if (Index == -1)                                                      // statically dead (OldNum >= 0); kept for fidelity
        {
            if (Minigame96_LogEnabled(4))
            {
                Minigame96_Logf(5, TEXT("AFortMinigame::AssignPlayerToAvailableBucket - Failed to find a bucket for player %s (%s); moving to spectate."),
                                *Minigame96_PlayerIdString(Player), *Mode);
            }
            Minigame96_MoveToSpectate(this, Player);                          // 0x14209b5a0
            Minigame96_RefreshBuckets(this);                                  // 0x1420a9a30 (LAB_142083ddb)
            return Index;
        }
    }

    // Join: PlayerBuckets[Index].PlayerIds.Add(Player->UniqueId)  (native copy-constructs 0x28 FUniqueNetIdRepl from Player+0x248, bumps shared ref)
    PlayerBuckets[Index].PlayerIds.Add(Player->GetUniqueId());
    if (Minigame96_LogEnabled(4))
    {
        Minigame96_Logf(5, TEXT("AFortMinigame::AssignPlayerToAvailableBucket - Moved player %s to bucket %d (%s)"),
                        *Minigame96_PlayerIdString(Player), Index, *Mode);
    }
    Minigame96_RefreshBuckets(this);                                          // 0x1420a9a30
    return Index;                                                             // Mode freed by FString dtor (FMemory_Free)
}
