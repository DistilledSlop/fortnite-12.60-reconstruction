// CONFIDENCE: HIGH. Complete original body; build/runtime pending.
// Original 141a14f80; inherited unchanged by vehicle and remote pawn families.
AFortPawn::GetOwnedGameplayTags(TagContainer);
if (GetLocalRole() == ROLE_Authority)
{
    if (AFortPlayerControllerAthena* PC = Cast<AFortPlayerControllerAthena>(GetController()))
    {
        if (UFortControllerComponent_PerkSystem* Perks = PC->FindComponentByClass<UFortControllerComponent_PerkSystem>())
        {
            const FGameplayTag FactionTag = Perks->NativeGetFactionTag141b46360();
            if (FactionTag.IsValid())
            {
                TagContainer.AddTag(FactionTag);
            }
        }
    }
}
