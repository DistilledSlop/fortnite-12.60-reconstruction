// CONFIDENCE: HIGH
// 0x1419b0920, with its iterator specialized to the proven PlayerState context.
struct FPlayerStateMutatorIterator
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

    FPlayerStateMutatorIterator(AFortPlayerStateAthena* PlayerState, AFortGameStateAthena* GameState)
        : Context(PlayerState)
    {
        if (PlayerState)
        {
            // This concrete class implements IFortMutatorContext; the generic
            // iterator's non-interface fallback is unreachable for this input.
            PlayerState->NativeAppendContextOwners(Owners);
        }
        if (GameState)
        {
            Owners.AddUnique(GameState);
        }
        SelectOwner(OwnerIndex);
    }

    ~FPlayerStateMutatorIterator()
    {
        if (Current)
        {
            Current->NativeSetMutatorContext(nullptr);
            Current = nullptr;
        }
    }
};

bool Result = false;
FPlayerStateMutatorIterator Iterator(PlayerState, GameState);
while (Iterator.Current)
{
    bool Value = false;
    const uint8 Control = Iterator.Current->NativeQueryInfiniteAmmo(Value, WeaponDefinition);
    Result |= Value;
    if (Control == 2)
    {
        break;
    }
    Iterator.SelectCandidate(Iterator.CandidateIndex + 1);
}
return Result;
