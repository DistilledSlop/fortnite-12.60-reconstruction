// CONFIDENCE: medium
// 0x142654460. Draft depends on the unresolved NativeObservers_0x218 field and
// UFortVisibilityComponent::NativeEvaluateVisibility virtual contract.
if (!Item.VisibilityComponent)
{
    return;
}

const uint16 OldMask = Item.TeamVisibilityFlag;
Item.TeamVisibilityFlag = 0;
for (AActor* Observer : NativeObservers_0x218)
{
    IFortTeamActorInterface* TeamActor = Observer ? Cast<IFortTeamActorInterface>(Observer) : nullptr;
    if (!TeamActor && Observer && Observer->GetInstigator())
    {
        TeamActor = Cast<IFortTeamActorInterface>(Observer->GetInstigator());
    }
    const uint8 Team = TeamActor ? TeamActor->NativeGetTeam163() : 0;
    const uint32 Bit = uint32(1) << (Team & 31);
    if ((Bit & Item.TeamVisibilityFlag) == 0 && Item.Actor)
    {
        const bool bVisible = Item.VisibilityComponent->NativeEvaluateVisibility(Observer, bForce);
        const uint16 NewMask = bVisible
            ? uint16(Item.TeamVisibilityFlag | Bit)
            : uint16(Item.TeamVisibilityFlag & ~Bit);
        Item.TeamVisibilityFlag = NewMask;
        if (OldMask != NewMask)
        {
            Item.VisibilityComponent->OnVisibilityChangedEvent.Broadcast(Item.Actor, Team, (Bit & NewMask) != 0);
            VisibilityInfoArray.MarkItemDirty(Item);
        }
    }
}
