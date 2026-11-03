// Original address: 0x141b9edb0.
// CONFIDENCE: HIGH
// Virtual slot 0x7a8 of the ABuildingActor vtable 0x145efb310 (class ctor 0x141b86f60); see building_vitals_runtime/README.md.
if (BuildingAttributeSet != nullptr)
{
	return BuildingAttributeSet->Health.GetCurrentValue();
}
return GetMaxHealth();
