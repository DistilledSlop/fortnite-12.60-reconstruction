// CONFIDENCE: HIGH. Original141b462b0; preserve player-state ability-system dispatch.
const AFortPlayerControllerAthena* PC = Cast<AFortPlayerControllerAthena>(GetOwner());
if (PC)
{
    if (AFortPlayerStateAthena* PS = PC->GetPlayerState<AFortPlayerStateAthena>())
    {
        return PS->GetAbilitySystemComponent();
    }
}
return nullptr;
