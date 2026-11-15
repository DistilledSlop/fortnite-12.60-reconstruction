// Blueprint-visible health getter. Retail exposes GetHealth/GetMaxHealth/GetHealthPercent as BlueprintCallable functions, and
// Prop_Athena_StaticGenerator_C.StormHealthModGenerator calls GetHealth through a name lookup: a class without the function is
// a fatal error (server crash 2026-11-15 21:28:46, ScriptCore.cpp:1268 FindFunctionChecked).
// Original address: 0x1427327f0.
// CONFIDENCE: LOW
// The address is the reflected UFunction create function; the body address is unknown. Health divided by max health.
const float MaxHealthValue = GetMaxHealth();
if (MaxHealthValue > 0.0f)
{
	return GetHealth() / MaxHealthValue;
}
return 0.0f;
