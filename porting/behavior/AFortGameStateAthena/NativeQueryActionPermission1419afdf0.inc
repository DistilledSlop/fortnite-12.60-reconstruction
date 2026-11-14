// CONFIDENCE: HIGH. Complete original body; native callback family inventoried.
// Complete 0x1419afdf0; callback slot 0x978 and playlist fallback.
struct FActionPermissionMutatorIterator
{
    TArray<UObject*> Owners;
    TWeakObjectPtr<UObject> Context;
    int32 OwnerIndex = 0;
    int32 CandidateIndex = 0;
    TArray<AFortGameplayMutator*> Candidates;
    AFortAthenaMutator* Current = nullptr;

    void SelectOwner(int32 Index)
    {
        QUICK_SCOPE_CYCLE_COUNTER(STAT_Fort_Mutator_Iterator);
        OwnerIndex = Index;
        if (!Owners.IsValidIndex(Index))
        {
            Current = nullptr;
            return;
        }
        Candidates.Reset();
        UObject* Owner = Owners[Index];
        if (Owner)
        {
            QUICK_SCOPE_CYCLE_COUNTER(STAT_Fort_Mutator_Interface);
            if (IFortMutatorOwner* Interface = Cast<IFortMutatorOwner>(Owner))
            {
                Interface->NativeAppendOwnerMutators(Candidates, Context.Get());
            }
            else
            {
                UFortMutatorListComponent* Component = static_cast<UFortMutatorListComponent*>(
                    static_cast<AActor*>(Owner)->FindComponentByClass(UFortMutatorListComponent::StaticClass()));
                if (Component)
                {
                    Candidates.Append(Component->Mutators);
                }
            }
        }
        if (Candidates.Num() < 1)
        {
            SelectOwner(OwnerIndex + 1);
        }
        else
        {
            SelectCandidate(0);
        }
    }

    void SelectCandidate(int32 Index)
    {
        for (;;)
        {
            CandidateIndex = Index;
            if (Current)
            {
                Current->NativeSetMutatorContext(nullptr);
                Index = CandidateIndex;
            }
            if (!Candidates.IsValidIndex(Index))
            {
                SelectOwner(OwnerIndex + 1);
                return;
            }
            Current = Cast<AFortAthenaMutator>(Candidates[Index]);
            if (Current && (Current->bMutatorActiveInternal || Current->bMutatorActive))
            {
                Current->NativeSetMutatorContext(Context.Get());
                return;
            }
            Index = CandidateIndex + 1;
        }
    }

    FActionPermissionMutatorIterator(AActor* ContextActor, AFortGameStateAthena* GameState)
        : Context(ContextActor)
    {
        if (ContextActor)
        {
            if (IFortMutatorContext* Interface = Cast<IFortMutatorContext>(ContextActor))
            {
                Interface->NativeAppendContextOwners(Owners);
            }
            else if (AFortVolumeManager* Manager = AFortVolumeManager::NativeResolveVolumeManager(ContextActor))
            {
                if (AFortVolume* ActorVolume = Manager->NativeFindActorVolume(ContextActor))
                {
                    Owners.AddUnique(ActorVolume);
                }
            }
        }
        if (GameState)
        {
            Owners.AddUnique(GameState);
        }
        SelectOwner(OwnerIndex);
    }

    ~FActionPermissionMutatorIterator()
    {
        if (Current)
        {
            Current->NativeSetMutatorContext(nullptr);
            Current = nullptr;
        }
    }
};


bool bAllowed = false;
{
    FActionPermissionMutatorIterator Iterator(nullptr, this);
    while (Iterator.Current)
    {
        const uint8 Control = Iterator.Current->NativeQueryActionPermission978(bAllowed, ActionTag);
        if (Control == 2)
        {
            return bAllowed; // Destructor clears current context before returning.
        }
        Iterator.SelectCandidate(Iterator.CandidateIndex + 1);
    }
} // Destructor runs before playlist lookup.
const UFortPlaylistAthena* Playlist = GetCurrentPlaylistAthena();
const bool bPlaylistAllows = Playlist && Playlist->AlwaysAllowedNativeActions.HasTag(ActionTag);
return bAllowed | bPlaylistAllows;
