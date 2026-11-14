// CONFIDENCE: HIGH. Original14160fd30; stop tests explicit tags only.
FGameplayTagContainer AbilityTags(ActionTag);
FGameplayTagContainer EmptyBlockTags;
FGameplayTagContainer EmptyCancelTags;
if (NativeActiveActionTags.HasTagExact(ActionTag))
{
    NativeActiveActionTags.RemoveTag(ActionTag);
    ApplyAbilityBlockAndCancelTags(AbilityTags, nullptr, false, EmptyBlockTags, false, EmptyCancelTags);
    HandleChangeAbilityCanBeCanceled(AbilityTags, nullptr, false);
    return;
}
if (UE_LOG_ACTIVE(LogFort, Verbose))
{
    const FString ActionName = ActionTag.ToString();
    const FString RoleName = UEnum::GetValueAsString(TEXT("Engine.ENetRole"), OwnerActor->GetLocalRole());
    UE_LOG(LogFort, Verbose, TEXT("Native Action Role(%s) Can't stop %s, not active!"), *RoleName, *ActionName);
}
