// CONFIDENCE: high; complete native capture and reflected data contracts.
// Native 0x1420b6d20: filters null and pending-kill entries from ActiveObjectives.
TArray<AFortObjectiveBase*> Result;
for (AFortObjectiveBase* Objective : ActiveObjectives)
{
    if (IsValid(Objective))
    {
        Result.Add(Objective);
    }
}
return Result;
