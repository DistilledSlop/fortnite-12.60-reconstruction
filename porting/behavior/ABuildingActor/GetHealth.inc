// Blueprint-visible health getter. Retail exposes GetHealth/GetMaxHealth/GetHealthPercent as BlueprintCallable functions, and
// Prop_Athena_StaticGenerator_C.StormHealthModGenerator calls GetHealth through a name lookup: a class without the function is
// a fatal error (server crash 2026-11-15 21:28:46, ScriptCore.cpp:1268 FindFunctionChecked).
// Original address: 0x141b9edb0.
// CONFIDENCE: MEDIUM
// Reads BuildingAttributeSet->Health when the set exists, otherwise the full max health (undamaged lazy actor).
const UFortBuildingActorSet* AttributeSet = BuildingAttributeSet;
if (AttributeSet != nullptr)
{
	return FMath::Max(0.0f, AttributeSet->Health.GetCurrentValue());
}
return GetMaxHealth();
