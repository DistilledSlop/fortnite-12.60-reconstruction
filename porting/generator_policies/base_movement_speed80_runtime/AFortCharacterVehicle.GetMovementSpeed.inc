// CONFIDENCE: high
// Original 0x141945ff0. 100 is the explicit original return.
if (!OstrichSet) return 100.0f;
if (Style == 0) return OstrichSet->WalkSpeed.GetCurrentValue();
if (Style == 3) return OstrichSet->SprintSpeed.GetCurrentValue();
return 100.0f;
