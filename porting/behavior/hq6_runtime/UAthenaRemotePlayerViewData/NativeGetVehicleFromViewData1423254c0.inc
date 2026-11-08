// Helper 0x1423254c0 (UAthenaRemotePlayerViewData). Asm evidence:
//   [this+0x1578] (null -> return 0); IsA AFortAthenaVehicle (StaticClass 0x1428381e0, class-tree check) -> object, else 0.
// Callers from other classes pass objects whose type is not verified; the host type is taken from the GetVehicle caller.
// CONFIDENCE: MEDIUM
UObject* SelectedVehicle = *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(ViewData) + 0x1578);
if (SelectedVehicle == nullptr)
{
    return nullptr;
}
if (SelectedVehicle->IsA(AFortAthenaVehicle::StaticClass()))
{
    return SelectedVehicle;
}
return nullptr;
