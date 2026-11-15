// Blueprint-visible health getter. Retail exposes GetHealth/GetMaxHealth/GetHealthPercent as BlueprintCallable functions, and
// Prop_Athena_StaticGenerator_C.StormHealthModGenerator calls GetHealth through a name lookup: a class without the function is
// a fatal error (server crash 2026-11-15 21:28:46, ScriptCore.cpp:1268 FindFunctionChecked).
// Original address: 0x141ba73d0.
// CONFIDENCE: MEDIUM
// MaxHealth from the attribute set when set, else the cached initialization value. The curve lookup the original falls back to
// is done by FortHarvest on first damage; before that an uninitialized actor reports the 100 fallback the harvest code uses.
const UFortBuildingActorSet* AttributeSet = BuildingAttributeSet;
if (AttributeSet != nullptr && AttributeSet->MaxHealth.GetCurrentValue() > 0.0f)
{
	return AttributeSet->MaxHealth.GetCurrentValue();
}
if (MaxHealthInitializationValue > 0.0f)
{
	return MaxHealthInitializationValue;
}
return 100.0f;
