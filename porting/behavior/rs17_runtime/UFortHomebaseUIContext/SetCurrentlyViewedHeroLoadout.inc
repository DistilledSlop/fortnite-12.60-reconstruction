// Original body address: 0x1450cbb30 (thunk 0x1452583a0)
// Offset evidence: weak pointer at object +0xd8 (unreflected gap, raw access). Get (0x142ee5dd0) compared with Loadout (RDX); equal -> return. Otherwise weak assign (0x142ee3b30) of Loadout, Get again, then delegate Broadcast at +0x80 (OnDifferentHeroLoadoutViewed) with packed params: Loadout at [RSP+0x20] (slot 0), Get result at [RSP+0x28] (slot 1).
// CONFIDENCE: MEDIUM
TWeakObjectPtr<UObject>* ViewedWeak = reinterpret_cast<TWeakObjectPtr<UObject>*>(reinterpret_cast<uint8*>(this) + 0xd8);
if (ViewedWeak->Get() == Loadout)
{
    return;
}
*ViewedWeak = Loadout;
UFortCampaignHeroLoadoutItem* ViewedLoadout = static_cast<UFortCampaignHeroLoadoutItem*>(ViewedWeak->Get());
OnDifferentHeroLoadoutViewed.Broadcast(Loadout, ViewedLoadout);
