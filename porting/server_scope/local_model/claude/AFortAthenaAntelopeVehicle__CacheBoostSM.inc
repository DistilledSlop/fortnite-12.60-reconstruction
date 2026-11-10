// CONFIDENCE: HIGH. Original exec 0x1427cf6f0; native body 0x1418bc370 (inlined). Parameter: InBoost (UStaticMeshComponent).
// Creates a dynamic material instance on element 0 of InBoost and caches it as the boost meter MID.
// The CreateDynamicMaterialInstance call is vtable slot 0x570, resolved to UPrimitiveComponent::CreateDynamicMaterialInstance.
if (InBoost)
{
    BoostMeterMID = InBoost->CreateDynamicMaterialInstance(0, nullptr, NAME_None);
}
