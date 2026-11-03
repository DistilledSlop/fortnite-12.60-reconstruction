// Typed reconstruction, not a standalone translation unit. Descriptive binding names
// below must be implemented using the original virtual slots; see HANDOFF.md.
// FRecoveredMutatorIterator is the source proposal in
// ../infinite_ammo_iterator_runtime/Proposal.cpp.in, including its destructor.
struct FMutatorValueQueryBindings
{
    static uint8 Health(AFortAthenaMutator* Mutator, float& Value); // vtable +0xaf0
    static uint8 Gold(AFortAthenaMutator* Mutator, float& Value); // +0xb10
    static uint8 SelfDamage(AFortAthenaMutator* Mutator, float& Value); // +0xb18
    static uint8 TrickScore(AFortAthenaMutator* Mutator, float& Value); // +0xb98

    // Read controller +0x220, then native IsA against class 0x1447c8410.
    // Return the original pointer on success, otherwise null. Do not add a
    // validity check or substitute the controller itself as iterator context.
    static UObject* ResolveTrickContext(AFortPlayerController* Controller);
};

float RecoveredSiphonHealth(AFortGameStateAthena* State, APawn* Pawn)
{
    float Value = 0.0f;
    FRecoveredMutatorIterator It(Pawn, State, true);
    while (AFortAthenaMutator* Mutator = It.Get())
    {
        if (FMutatorValueQueryBindings::Health(Mutator, Value)) break;
        It.Advance();
    }
    return Value;
}

float RecoveredSiphonGold(AFortGameStateAthena* State, APawn* Pawn)
{
    float Value = 0.0f;
    FRecoveredMutatorIterator It(Pawn, State, true);
    while (AFortAthenaMutator* Mutator = It.Get())
    {
        if (FMutatorValueQueryBindings::Gold(Mutator, Value)) break;
        It.Advance();
    }
    return Value;
}

float RecoveredSelfDamage(AFortGameStateAthena* State, APawn* Pawn)
{
    float Value = 0.0f;
    FRecoveredMutatorIterator It(Pawn, State, true);
    while (AFortAthenaMutator* Mutator = It.Get())
    {
        if (FMutatorValueQueryBindings::SelfDamage(Mutator, Value)) break;
        It.Advance();
    }
    return Value;
}

float RecoveredVehicleTrickScore(AFortGameStateAthena* State,
                                 AFortPlayerController* Controller, float InTrickScore)
{
    UObject* Context = FMutatorValueQueryBindings::ResolveTrickContext(Controller);
    FRecoveredMutatorIterator It(Context, State, true);
    while (AFortAthenaMutator* Mutator = It.Get())
    {
        float Candidate = InTrickScore;
        if (FMutatorValueQueryBindings::TrickScore(Mutator, Candidate)) return Candidate;
        It.Advance();
    }
    return InTrickScore;
}
