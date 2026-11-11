// CONFIDENCE: HIGH. Copies the BuildingSMActor members of one logical building group into the output array; an out-of-range index yields an empty array.
// Original addresses: exec 0x14275dc40; body helper 0x141c828e0.
LogicalBuildingActors.Reset();
if (!LogicalBuildingGroups.IsValidIndex(LogicalIdx))
{
	return;
}
for (const FWeakObjectPtr& WeakActor : LogicalBuildingGroups[LogicalIdx].BuildingActors)
{
	UObject* BuildingActor = WeakActor.Get();
	if (BuildingActor && BuildingActor->IsA(ABuildingSMActor::StaticClass()))
	{
		LogicalBuildingActors.Add(BuildingActor);
	}
}
