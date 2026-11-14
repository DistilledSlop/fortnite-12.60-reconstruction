// CONFIDENCE: HIGH. Complete original body; class lifecycle acceptance remains separate.
// PROPOSAL 0x1419aff90, reusing already emitted generic iterator contracts.
// New helper signature: bool AFortGameStateAthena::NativeEvaluateJumpPenaltyMutators1419aff90(ACharacter* Pawn)
// Binding requires virtual NativeQueryJumpPenaltyB38 on AFortAthenaMutator with two overrides below.
struct FJumpPenaltyMutatorIterator
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

    FJumpPenaltyMutatorIterator(AActor* ContextActor, AFortGameStateAthena* GameState)
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

    ~FJumpPenaltyMutatorIterator()
    {
        if (Current)
        {
            Current->NativeSetMutatorContext(nullptr);
            Current = nullptr;
        }
    }
};

bool bApply = true;
FJumpPenaltyMutatorIterator Iterator(Pawn, this);
while (Iterator.Current)
{
    const uint8 Control = Iterator.Current->NativeQueryJumpPenaltyB38(bApply);
    if (Control == 2) break;
    Iterator.SelectCandidate(Iterator.CandidateIndex + 1);
}
return bApply;
