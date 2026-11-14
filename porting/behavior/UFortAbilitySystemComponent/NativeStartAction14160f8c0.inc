// CONFIDENCE: HIGH. Original14160f8c0; containers exist before the duplicate check.
FGameplayTagContainer AbilityTags(ActionTag);
FGameplayTagContainer EmptyBlockTags;
FGameplayTagContainer EmptyCancelTags;
if (NativeActiveActionTags.HasTag(ActionTag))
{
    if (UE_LOG_ACTIVE(LogFort, Log))
    {
        const FString ActionName = ActionTag.ToString();
        const FString RoleName = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), OwnerActor->GetLocalRole());
        UE_LOG(LogFort, Log, TEXT("Native Action Role(%s) Can't start %s, already active!"), *RoleName, *ActionName);
    }
    return;
}
NativeActiveActionTags.AddTag(ActionTag);
ApplyAbilityBlockAndCancelTags(AbilityTags, nullptr, true, EmptyBlockTags, true, EmptyCancelTags);
HandleChangeAbilityCanBeCanceled(AbilityTags, nullptr, true);
