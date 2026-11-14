// CONFIDENCE: HIGH. 0x14199ce60, with the +0xba0 native family inventoried.
// Iterator shape is shared with the recovered 14198b790 implementation.
struct FOpenParachuteMutatorIterator
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

    FOpenParachuteMutatorIterator(AActor* ContextActor, AFortGameStateAthena* GameState)
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

    ~FOpenParachuteMutatorIterator()
    {
        if (Current)
        {
            Current->NativeSetMutatorContext(nullptr);
            Current = nullptr;
        }
    }
};


{
    FOpenParachuteMutatorIterator Iterator(nullptr, this);
    while (Iterator.Current)
    {
        if (Iterator.Current->NativeConsumeParachuteOpenBA0(bFromInput, Pawn))
        {
            return true; // iterator destructor clears current mutator context
        }
        Iterator.SelectCandidate(Iterator.CandidateIndex + 1);
    }
}
return false;
