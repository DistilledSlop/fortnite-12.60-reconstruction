// CONFIDENCE: LOW. Original exec 0x14296c830 (thunk); body 0x141ef1630. Query class 0x142a75f10 (AFortPlayerPawn) with flags 5.
// Returns the AFortPlayerPawn of the world whose root location is nearest to Position (squared distance, first accepted candidate seeds the best).
// The eligibility filter (FortPawnQueryAccepts) is only partly decoded: dead/downed and owner/anchor tests. See deps/FortPawnQueryAccepts.json.
UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
if (World == nullptr)
{
	return nullptr;
}

UObject* Anchor = FortPawnQueryAnchor(World, AFortPlayerPawn::StaticClass(), 5);
AFortPlayerPawn* ClosestPawn = nullptr;
float ClosestDistanceSquared = 0.0f;

for (TActorIterator<AFortPlayerPawn> It(World); It; ++It)
{
	AFortPlayerPawn* Pawn = *It;
	if (!FortPawnQueryAccepts(Pawn, Anchor, 5))
	{
		continue;
	}

	const USceneComponent* Root = Pawn->GetRootComponent();
	const FVector PawnLocation = Root != nullptr ? Root->GetComponentLocation() : FVector::ZeroVector;
	const float DistanceSquared = FVector::DistSquared(PawnLocation, Position);
	if (ClosestPawn == nullptr || DistanceSquared < ClosestDistanceSquared)
	{
		ClosestPawn = Pawn;
		ClosestDistanceSquared = DistanceSquared;
	}
}

return ClosestPawn;
