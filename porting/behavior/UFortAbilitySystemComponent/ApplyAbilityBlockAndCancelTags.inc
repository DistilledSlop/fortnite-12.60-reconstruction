// CONFIDENCE: HIGH. Complete original 0x1415e9850; UE build/runtime pending.
FGameplayTagContainer ExpandedBlockTags(BlockTags);
FGameplayTagContainer ExpandedCancelTags(CancelTags);
UFortGameData* CommonGameData = UFortAssetManager::NativeGetFortAssetManager().NativeGetCommonGameData();
CommonGameData->NativeExpandAbilityRelationships141e5b330(AbilityTags, &ExpandedBlockTags, &ExpandedCancelTags);
Super::ApplyAbilityBlockAndCancelTags(AbilityTags, RequestingAbility, bEnableBlockTags,
    ExpandedBlockTags, bExecuteCancelTags, ExpandedCancelTags);

const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
if (!ActorInfo || !ActorInfo->OwnerActor.IsValid()) return;
AController* Controller = Cast<AController>(ActorInfo->OwnerActor.Get()->GetOwner());
if (!Controller) return;

if (FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x668).MatchesAny(ExpandedBlockTags))
    Controller->SetIgnoreMoveInput(bEnableBlockTags);
if (FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x670).MatchesAny(ExpandedBlockTags))
    Controller->SetIgnoreLookInput(bEnableBlockTags);

AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(ActorInfo->AvatarActor.Get());
if (!Pawn) return;
if (AFortWeapon* Weapon = Pawn->CurrentWeapon)
{
    if (FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x768).MatchesAny(ExpandedCancelTags))
        Weapon->NativeSetTargeting1426b3730(false);
}
if (Pawn->bIsCrouched &&
    FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x698).MatchesAny(ExpandedCancelTags))
    Pawn->NativeCancelCrouch142336830();
