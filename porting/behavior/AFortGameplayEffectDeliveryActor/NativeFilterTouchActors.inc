// CONFIDENCE: high
// Original address: 0x1415f4550
// 0x1415f4550 / 0x1415e4bb0 / 0x14160eb90. The seen set spans the reverse pass.
FCollisionQueryParams FilterInfo(FName(TEXT("FortTargetFilter_FilterInfo")), false);
const bool bFilterRelationships = Filter.bExcludePawnFriends || Filter.bExcludeFriendlyAI ||
    Filter.bExcludePawnEnemies || Filter.bExcludeNonPawnFriends || Filter.bExcludeNonPawnEnemies;
AFortGameStateZone* ZoneState = bFilterRelationships ? Cast<AFortGameStateZone>(World->GetGameState()) : nullptr;
UClass* ClassFilter = Filter.ActorClassFilter.Get();
UClass* TypeClass = nullptr;
switch (Filter.ActorTypeFilter)
{
case EFortTargetSelectionFilter::Pawns:
case EFortTargetSelectionFilter::Instigator: TypeClass = APawn::StaticClass(); break;
case EFortTargetSelectionFilter::Buildings: TypeClass = ABuildingSMActor::StaticClass(); break;
case EFortTargetSelectionFilter::Walls: TypeClass = ABuildingWall::StaticClass(); break;
case EFortTargetSelectionFilter::Traps: TypeClass = ABuildingTrap::StaticClass(); break;
case EFortTargetSelectionFilter::Players: TypeClass = AFortPlayerPawn::StaticClass(); break;
case EFortTargetSelectionFilter::AIPawns: TypeClass = AFortAIPawn::StaticClass(); break;
default: break;
}
if (TypeClass && (!ClassFilter || !ClassFilter->IsChildOf(TypeClass)))
{
    ClassFilter = TypeClass;
}
APawn* InstigatorPawn = Requester ? Requester->GetInstigator() : nullptr;
AFortVolumeManager* VolumeManager = nullptr;
AFortVolume* InstigatorVolume = nullptr;
if (Requester)
{
    AFortGameStateAthena* AthenaState = Cast<AFortGameStateAthena>(World->GetGameState());
    if (NativeUseVolumeTargetFiltering163() && AthenaState)
    {
        UFortPlaylistAthena* Playlist = AthenaState->GetCurrentPlaylistAthena();
        FFortNativeGameplayTagsRuntime& Tags = FFortNativeGameplayTagsRuntime::Get();
        if (Playlist && (Playlist->GameplayTagContainer.HasTag(Tags.AthenaCreative()) ||
            Playlist->GameplayTagContainer.HasTag(Tags.AthenaPlayground())) && InstigatorPawn)
        {
            VolumeManager = AFortVolumeManager::NativeResolveVolumeManager(InstigatorPawn);
            if (VolumeManager)
            {
                InstigatorVolume = VolumeManager->NativeFindVolumeAtLocation(InstigatorPawn->GetActorLocation());
            }
        }
    }
}
TSet<AActor*> SeenActors;
const auto ResolveMinigame = [](APawn* Pawn) -> AFortMinigame*
{
    APlayerState* PlayerState = Pawn ? Pawn->GetPlayerState() : nullptr;
    if (!PlayerState)
    {
        return nullptr;
    }
    UWorld* PlayerWorld = PlayerState->GetWorld();
    AFortGameStateAthena* State = PlayerWorld ? Cast<AFortGameStateAthena>(PlayerWorld->GetGameState()) : nullptr;
    AFortVolumeManager* Manager = State ? State->NativeGetVolumeManager() : nullptr;
    AFortVolume* Volume = Manager ? Manager->NativeFindPlayerVolume(PlayerState) : nullptr;
    return UFortMinigameVolumeComponent::NativeResolveMinigameFromVolume(Volume);
};
const auto ShouldExclude = [&](AActor* Candidate)
{
    QUICK_SCOPE_CYCLE_COUNTER(STAT_Fort_ShouldFilterOutActor);
    if (!IsValid(Candidate) || (Filter.ActorTypeFilter == EFortTargetSelectionFilter::Damageable &&
        !Candidate->CanBeDamaged()) || SeenActors.Contains(Candidate))
    {
        return true;
    }
    if (ClassFilter && !Candidate->IsA(ClassFilter))
    {
        if (!UFortGameInstance::NativeIsGameMode141ed8de0(Candidate, 0) || ClassFilter != APawn::StaticClass())
        {
            return true;
        }
        IGameplayTagAssetInterface* TagInterface = Cast<IGameplayTagAssetInterface>(Candidate);
        if (!TagInterface)
        {
            return true;
        }
        static FGameplayTag FakePawnTag;
        UFortGameData* GameData = UFortAssetManager::NativeGetFortAssetManager().NativeGetCommonGameData();
        UFortGameplayTagData* TagData = nullptr;
        if (!GameData->GameplayTagData.IsNull())
        {
            TagData = GameData->GameplayTagData.Get();
            if (!TagData)
            {
                TagData = Cast<UFortGameplayTagData>(FortLoadDelayedObject(GameData->GameplayTagData.ToSoftObjectPath(), true));
            }
            FFortAssets::Get().TrackAsset(TagData);
        }
        if (TagData)
        {
            FakePawnTag = TagData->FakePawnTag;
        }
        if (!FakePawnTag.IsValid() || !TagInterface->HasMatchingGameplayTag(FakePawnTag))
        {
            return true;
        }
    }
    if (Candidate->IsA<ALandscapeProxy>())
    {
        return true;
    }
    if (VolumeManager && VolumeManager->NativeFindVolumeAtLocation(Candidate->GetActorLocation()) != InstigatorVolume)
    {
        return true;
    }
    SeenActors.Add(Candidate);
    APawn* EffectiveInstigator = InstigatorPawn;
    if (Filter.bUseTrapsOwningPawnAsInstigator && Requester)
    {
        if (ABuildingTrap* Trap = Cast<ABuildingTrap>(Requester))
        {
            APawn* OwningPawn = nullptr;
            if (Trap->OwnerPersistentID != -1)
            {
                if (UWorld* TrapWorld = Trap->GetWorld())
                {
                    for (TActorIterator<APawn> It(TrapWorld); It; ++It)
                    {
                        AFortPlayerState* PlayerState = Cast<AFortPlayerState>(It->GetPlayerState());
                        if (PlayerState && PlayerState->WorldPlayerId == Trap->OwnerPersistentID)
                        {
                            OwningPawn = *It;
                            break;
                        }
                    }
                }
            }
            if (OwningPawn)
            {
                EffectiveInstigator = OwningPawn;
                if (Filter.bExcludeInstigator)
                {
                    TArray<APawn*> Occupants;
                    NativeGetVehicleOccupants163(Candidate, Occupants);
                    if (Occupants.Contains(OwningPawn))
                    {
                        return true;
                    }
                }
            }
        }
    }
    if (Candidate == EffectiveInstigator)
    {
        if (Filter.bExcludeInstigator)
        {
            return true;
        }
        if (Candidate == Requester && Filter.bExcludeRequester)
        {
            return true;
        }
    }
    else if (Candidate == Requester)
    {
        if (Filter.bExcludeRequester)
        {
            return true;
        }
    }
    else if (bFilterRelationships)
    {
        const bool bFortPawn = Candidate->IsA<AFortPawn>();
        bool bFriends = ZoneState && ZoneState->NativeAreFriendlyActors163(Requester, Candidate);
        bool bExclude;
        if (!bFortPawn)
        {
            TArray<APawn*> Occupants;
            if (NativeGetVehicleOccupants163(Candidate, Occupants) && ZoneState)
            {
                bFriends = Occupants.Num() > 0;
                for (APawn* Occupant : Occupants)
                {
                    if (!ZoneState->NativeAreFriendlyActors163(Requester, Occupant))
                    {
                        bFriends = false;
                        break;
                    }
                }
            }
            bExclude = bFriends ? Filter.bExcludeNonPawnFriends : Filter.bExcludeNonPawnEnemies;
        }
        else
        {
            bExclude = bFriends ? (Filter.bExcludePawnFriends ||
                (Filter.bExcludeFriendlyAI && Candidate->IsA<AFortAIPawn>())) : Filter.bExcludePawnEnemies;
        }
        AFortMinigame* Minigame = ResolveMinigame(EffectiveInstigator);
        if (!(Minigame && Minigame->NativeAllowsFriendlyFire163(Cast<AFortPawn>(EffectiveInstigator)) && bExclude && bFriends))
        {
            if (AFortGameStateAthena* AthenaState = Cast<AFortGameStateAthena>(ZoneState))
            {
                FGameplayTagContainer EmptyTags;
                if (AthenaState->NativeAllowsFriendlyFire163(EmptyTags, Cast<AFortPawn>(EffectiveInstigator)) &&
                    Filter.bConsiderFriendlyFireWhenExcludingFriends && bExclude && bFriends)
                {
                    bExclude = false;
                }
            }
            if (bExclude)
            {
                return true;
            }
        }
    }
    if (AFortPawn* Pawn = Cast<AFortPawn>(Candidate))
    {
        if ((Pawn->IsDead() && !Pawn->bIsDBNO) || (Filter.bExcludeDBNOPawns && Pawn->bIsDBNO))
        {
            return true;
        }
    }
    if (Filter.bExcludeAllAI && Candidate->IsA<AFortAIPawn>())
    {
        return true;
    }
    if (Filter.bExcludeWaterVolumes && (Candidate->IsA<AWaterVolume_Shallow>() || Candidate->IsA<AWaterVolume_Deep>()))
    {
        return true;
    }
    if (Filter.bExcludeWaterBodies && Candidate->IsA<AFortWaterBodyActor>())
    {
        return true;
    }
    if (NativeExcludeVehicleTarget163(Filter, Candidate, Requester))
    {
        return true;
    }
    if ((Filter.ActorTypeFilter == EFortTargetSelectionFilter::Pawns || Filter.ActorTypeFilter == EFortTargetSelectionFilter::Players) &&
        Candidate->IsA<APhysicsObject>())
    {
        return true;
    }
    if (Filter.bExcludeNonPlayerBuiltPieces || Filter.bExcludePlayerBuiltPieces)
    {
        if (ABuildingActor* BuildingActor = Cast<ABuildingActor>(Candidate))
        {
            if ((Filter.bExcludeNonPlayerBuiltPieces && !BuildingActor->IsPlayerBuilt()) ||
                (Filter.bExcludePlayerBuiltPieces && BuildingActor->IsPlayerBuilt()))
            {
                return true;
            }
        }
    }
    if (Filter.bExcludeNonBGABuildings && Candidate->IsA<ABuildingActor>() && !Candidate->IsA<ABuildingGameplayActor>())
    {
        return true;
    }
    if (Filter.bExcludeProjectiles && Candidate->IsA<AFortProjectileBase>())
    {
        return true;
    }
    if (Filter.bExcludeWithoutNavigationCorridor && Requester)
    {
        FVector HitLocation;
        if (UNavigationSystemV1::NavigationRaycast(World, Requester->GetActorLocation(), Candidate->GetActorLocation(),
            HitLocation, URecastFilter_UseDefaultArea::StaticClass(), nullptr))
        {
            return true;
        }
    }
    if (Filter.ExclusionGameplayTags.Num() != 0)
    {
        if (IGameplayTagAssetInterface* Tags = Cast<IGameplayTagAssetInterface>(Candidate))
        {
            if (Tags->HasAnyMatchingGameplayTags(Filter.ExclusionGameplayTags))
            {
                return true;
            }
        }
    }
    return false;
};
for (int32 Index = Actors.Num() - 1; Index >= 0; --Index)
{
    if (ShouldExclude(Actors[Index]))
    {
        Actors.RemoveAt(Index, 1, true);
    }
}
