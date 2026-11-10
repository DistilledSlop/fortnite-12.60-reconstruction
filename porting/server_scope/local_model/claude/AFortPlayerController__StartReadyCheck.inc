// CONFIDENCE: HIGH - exec thunk forwards to the native ready-check helper.
// Original exec 0x142a4f7a0, helper 0x1423b8db0. FFrame read order: QueryText, YesText, NoText, PercentageToPass.
// The thunk's FText copies are plain by-value handoffs; the helper releases them on return.
return NativeStartReadyCheck(QueryText, YesText, NoText, PercentageToPass);
