// CONFIDENCE: HIGH
// Original 0x141738f90. Foundation is nonnull by the original caller contract.
if (bMarkBuildingFoundationDirty)
{
    const ABuildingFoundation* BuildingFoundation = Foundation;
    const USceneComponent* Root = BuildingFoundation->GetRootComponent();
    const FTransform& Transform = Root ? Root->GetComponentTransform() : FTransform::Identity;
    AddDirtyArea(BuildingFoundation->NativeGetFoundationBounds().TransformBy(Transform), ENavigationDirtyFlag::All);
}
