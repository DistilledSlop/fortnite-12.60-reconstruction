// CONFIDENCE: high
// Native 0x14231a530; wrapper 0x142a1bf00.
UFortControlResistanceSet* ResistanceSet = ControlResistanceSet;
return ResistanceSet ? ResistanceSet->ControlResistance.GetCurrentValue() : 0.0f;
