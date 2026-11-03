// CONFIDENCE: high; complete native capture and reflected data contracts.
// Native 0x1420bbd80 delegates to 0x1420bb9d0 and returns 5 on a miss.
const AFortObjectiveBase* Objective = GetObjectiveByHandle(ObjectiveHandle);
return Objective ? Objective->ObjectiveStatus : EFortObjectiveStatus::Max_None;
