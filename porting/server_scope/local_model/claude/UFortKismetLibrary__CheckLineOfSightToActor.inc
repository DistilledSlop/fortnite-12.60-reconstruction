// CONFIDENCE: LOW. Visibility test from SourcePos to Target. Returns false when Target has no root component. Otherwise it queries from SourcePos to the root's location, ignoring Source. If that is blocked and the root is a UPrimitiveComponent, it queries again to the root's bounds origin. Returns true when either query is clear. The pawn-class flag (Target is an AFortPlayerPawn) is passed to the query; its exact effect is not named.
// Original address: 0x142967c90 (exec). Native body: 0x141ee8580. Query: 0x141ee3010. Class gates: 0x142a75f10 (AFortPlayerPawn), 0x1447c8590 (UPrimitiveComponent).
if (Target == nullptr)
{
    return false;
}
USceneComponent* TargetRoot = Target->GetRootComponent();
if (TargetRoot == nullptr)
{
    return false;
}
const bool bTargetIsPawn = Target->IsA(AFortPlayerPawn::StaticClass());
FCollisionQueryParams Params(SCENE_QUERY_STAT(CheckLineOfSightToActor), true);
if (Source != nullptr)
{
    Params.AddIgnoredActor(Source);
}
bool bVisible = LineOfSightQuery(SourcePos, TargetRoot->GetComponentLocation(), Params, bTargetIsPawn);
if (!bVisible && TargetRoot->IsA(UPrimitiveComponent::StaticClass()))
{
    bVisible = LineOfSightQuery(SourcePos, TargetRoot->Bounds.Origin, Params, bTargetIsPawn);
}
return bVisible;
