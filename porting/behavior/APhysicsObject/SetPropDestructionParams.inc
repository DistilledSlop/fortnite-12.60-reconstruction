// CONFIDENCE: high. Native target 0x141b537c0.
if (InBuildingPropClassesPhysicsObjectCanDestroy.Num() > 0)
{
    BuildingPropClassesPhysicsObjectCanDestroy = MoveTemp(InBuildingPropClassesPhysicsObjectCanDestroy);
}
if (InWallsPhysicsObjectCanDestroy.Num() > 0)
{
    WallsPhysicsObjectCanDestroy = MoveTemp(InWallsPhysicsObjectCanDestroy);
}
