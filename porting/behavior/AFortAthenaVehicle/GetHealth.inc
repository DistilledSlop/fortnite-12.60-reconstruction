// CONFIDENCE: high; original interface target reads the reflected current health attribute or zero.
{
    const UFortHealthSet* CurrentHealthSet = HealthSet;
    return CurrentHealthSet ? CurrentHealthSet->Health.GetCurrentValue() : 0.0f;
}
