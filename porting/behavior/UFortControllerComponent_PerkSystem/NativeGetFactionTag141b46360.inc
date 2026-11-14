// CONFIDENCE: HIGH. Complete original body; build/runtime pending.
// Original 141b46360. Snapshot refs are released before the ASC update.
if (!CachedFactionTag.IsValid())
{
    if (AFortPlayerControllerAthena* PC = Cast<AFortPlayerControllerAthena>(GetOwner()))
    {
        if (UFortMcpProfileAthena* Profile = PC->AthenaProfile)
        {
            FMcpProfileItemArray Items;
            Profile->NativeGetProfileGroupSnapshot(TEXT("PlayerTech"), Items);
            if (Items.Num() == 0)
            {
                UE_LOG(LogFortControllerComponent_PerkSystem, Warning, TEXT("[AFortAthenaMutator_Infiltration::OnRep_MutatorData()] No Player Perk Items Founds?! No Faction Tag!!"));
            }
            for (const FMcpProfileItemPtr& Entry : Items)
            {
                UFortAccountItem* Item = Cast<UFortAccountItem>(Entry->ItemObject);
                if (Item && Item->IsFavorite())
                {
                    const FString TemplateIdCopy = Item->TemplateId;
                    const FName ItemName(TemplateIdCopy.Len(), *TemplateIdCopy, FNAME_Add);
                    const FGameplayTag* Found = ItemName.IsNone() ? nullptr : MutatorData.FactionItemMapping.Find(ItemName);
                    CachedFactionTag = Found ? *Found : FGameplayTag();
                    if (!CachedFactionTag.IsValid())
                    {
                        UE_LOG(LogFortControllerComponent_PerkSystem, Warning, TEXT("[AFortAthenaMutator_Infiltration::OnRep_MutatorData()] Tag from item [%s] is invalid!"), *ItemName.ToString());
                    }
                    break;
                }
            }
        }
    }
    if (CachedFactionTag.IsValid())
    {
        if (UAbilitySystemComponent* ASC = NativeGetAbilitySystemComponent141b462b0())
        {
            ASC->SetLooseGameplayTagCount(CachedFactionTag, 1);
        }
    }
}
return CachedFactionTag;
