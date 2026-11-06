// CONFIDENCE: high; complete native capture and reflected data contracts.
// Native 0x1420bb9d0: empty query never matches; parent tags count as matches.
if (ObjectiveHandle.IsEmpty())
{
    return nullptr;
}
for (AFortObjectiveBase* Objective : ActiveObjectives)
{
    if (Objective && Objective->ObjectiveHandle.HasAll(ObjectiveHandle))
    {
        return Objective;
    }
}
return nullptr;
