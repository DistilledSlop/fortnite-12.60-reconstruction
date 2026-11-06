// CONFIDENCE: high
// Original address: 0x142083430
// 0x142083430, including the generic active-mutator iterator and context cleanup.
const uint8 State = static_cast<uint8>(CurrentState);
if (State < 1 || State > 5)
{
    return false;
}
bool Result = bAllowFriendlyFire;
if (!Pawn)
{
    return Result;
}
struct FFriendlyFireMutatorIterator
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

    FFriendlyFireMutatorIterator(AActor* ContextActor, AFortGameStateAthena* GameState)
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
                if (AFortVolume* Volume = Manager->NativeFindActorVolume(ContextActor))
                {
                    Owners.AddUnique(Volume);
                }
            }
        }
        if (GameState)
        {
            Owners.AddUnique(GameState);
        }
        SelectOwner(OwnerIndex);
    }

    ~FFriendlyFireMutatorIterator()
    {
        if (Current)
        {
            Current->NativeSetMutatorContext(nullptr);
            Current = nullptr;
        }
    }
};

UWorld* World = GetWorld();
AFortGameStateAthena* GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
FFriendlyFireMutatorIterator Iterator(Pawn->GetPlayerState(), GameState);
while (Iterator.Current)
{
    bool Value;
    if (Iterator.Current->NativeQueryFriendlyFire163(Value))
    {
        Result = Value;
        break;
    }
    Iterator.SelectCandidate(Iterator.CandidateIndex + 1);
}
return Result;
