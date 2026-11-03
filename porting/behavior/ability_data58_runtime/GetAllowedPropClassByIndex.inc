// CONFIDENCE: HIGH
// Native 0x141626d10, reflected wrapper 0x1428fd500.
const TSoftClassPtr<ABuildingActor>* AllowedClass = AllowedPropClasses.IsValidIndex(AllowedPropIndex)
    ? &AllowedPropClasses[AllowedPropIndex] : nullptr;
return AllowedClass ? AllowedClass->Get() : nullptr;
