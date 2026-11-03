// CONFIDENCE: high
// Native 0x14231f210; wrapper 0x142a1c2d0.
UFortControlResistanceSet* ResistanceSet = ControlResistanceSet;
return ResistanceSet ? ResistanceSet->KnockbackThreshold.GetCurrentValue() : 0.0f;
