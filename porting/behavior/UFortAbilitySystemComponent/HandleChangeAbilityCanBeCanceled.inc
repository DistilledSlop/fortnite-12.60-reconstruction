// CONFIDENCE: HIGH. Original141600f50, ASC virtual +0x668.
const FGameplayAbilityActorInfo* ActorInfo = AbilityActorInfo.Get();
if (!ActorInfo) return;

AFortPlayerController* FortController = nullptr;
if (ActorInfo->OwnerActor.IsValid())
{
    FortController = Cast<AFortPlayerController>(ActorInfo->OwnerActor.Get()->GetOwner());
}
if (!FortController)
{
    if (!ActorInfo->AvatarActor.IsValid()) return;
    AFortPlayerPawn* Pawn = Cast<AFortPlayerPawn>(ActorInfo->AvatarActor.Get());
    if (!Pawn) return;
    AFortCharacterVehicle* Vehicle = Cast<AFortCharacterVehicle>(Pawn);
    if (!Vehicle) return;
    FortController = Cast<AFortPlayerController>(Vehicle->NativeGetLastController141947020());
    if (!FortController) return;
}

const FGameplayTag& MoveAction = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x668);
UFortGameData* GameData = UFortAssetManager::NativeGetFortAssetManager().NativeGetCommonGameData();
if (GameData->NativeAbilityRelationshipReferences141e6c4f0(AbilityTags, MoveAction))
{
    if (bCanBeCanceled)
        FortController->NativeStartMovementCancellableAction1423b8b70();
    else
        FortController->NativeEndMovementCancellableAction142378db0();
}
