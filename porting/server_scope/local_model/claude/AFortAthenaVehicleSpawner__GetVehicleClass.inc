// CONFIDENCE: MEDIUM. Original addresses: exec thunk 0x142841fd0, helper 0x141919180.
// Resolves the spawner's vehicle class through its item definition.
UObject* ItemDef = FortVehicleItemDef.LoadSynchronous();   // soft object at +0x220; helper 0x141907b90 (loads, load=1)
if (ItemDef == nullptr)
{
    return nullptr;
}
UClass* VehicleClass = nullptr;
TSoftClassPtr<UObject>& ClassPtr = ItemDef->VehicleClassSoft;   // soft class at ItemDef+0x818; helper 0x141907d50
VehicleClass = Cast<UClass>(ClassPtr.LoadSynchronous());
if (VehicleClass != nullptr && VehicleClass->IsChildOf(AActor::StaticClass()))
{
    return VehicleClass;
}
return nullptr;
