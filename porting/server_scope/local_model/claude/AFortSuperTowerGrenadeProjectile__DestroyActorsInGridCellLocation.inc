// CONFIDENCE: MEDIUM. Gathers the actors occupying one grid cell and queues them. Building actors go to both BuildingActorsToDestroy and BuildingActorsToIgnore. Floor and extra actors go to BuildingActorsToIgnore, and also to BuildingActorsToDestroy when bDestroyFloorsQuickly is false. With bDestroyFloorsQuickly set, a floor actor carrying the quick-destroy object flag is destroyed right away and not queued.
// Original addresses: exec 0x142aef190 (thunk; calls the body 0x141a74b30 directly). Helpers: 0x141c75940 (cell actor query, fills three weak-pointer lists; output mapping LOW), 0x143f6c200 (DestroyActorChecked).

TArray<TWeakObjectPtr<AActor>> CellBuildingActors;
TArray<TWeakObjectPtr<AActor>> CellFloorActors;
TArray<TWeakObjectPtr<AActor>> CellExtraActors;
NativeQueryGridCellActors(StructuralGrid, CellLocation, CellBuildingActors, CellFloorActors, CellExtraActors);

for (TWeakObjectPtr<AActor>& WeakBuildingActor : CellBuildingActors)
{
    AActor* BuildingActor = WeakBuildingActor.Get();
    if (BuildingActor == nullptr)
    {
        continue;
    }
    BuildingActorsToDestroy.Add(BuildingActor);
    BuildingActorsToIgnore.Add(BuildingActor);
}

TArray<TWeakObjectPtr<AActor>>* FloorLists[] = { &CellFloorActors, &CellExtraActors };
for (TArray<TWeakObjectPtr<AActor>>* FloorList : FloorLists)
{
    for (TWeakObjectPtr<AActor>& WeakFloorActor : *FloorList)
    {
        AActor* FloorActor = WeakFloorActor.Get();
        if (FloorActor == nullptr)
        {
            continue;
        }

        if (!bDestroyFloorsQuickly)
        {
            BuildingActorsToDestroy.Add(FloorActor);
        }
        else if (FloorActor->HasQuickDestroyFlag())
        {
            DestroyActorChecked(FloorActor, false, true);
            continue;
        }

        BuildingActorsToIgnore.Add(FloorActor);
    }
}
