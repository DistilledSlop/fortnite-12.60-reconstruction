// CONFIDENCE: LOW. Original exec 0x14296d600; body 0x141ef2aa0. Anchor lookup 0x140894360 with class 0x142a174f0 (AFortPawn), flags 5.
// Clears OutFortPawns, then appends every accepted, available AFortPawn whose root location is within Range of SourcePosition (inclusive).
// The eligibility filter (FortPawnQueryAccepts) and availability test (FortPawnIsUnavailable) are only partly decoded. See deps.
OutFortPawns.Reset();

UWorld* World = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
if (World == nullptr)
{
	return;
}

UObject* Anchor = FortPawnQueryAnchor(World, AFortPawn::StaticClass(), 5);
const float RangeSquared = Range * Range;

for (TActorIterator<AFortPawn> It(World); It; ++It)
{
	AFortPawn* Pawn = *It;
	if (!FortPawnQueryAccepts(Pawn, Anchor, 5) || FortPawnIsUnavailable(Pawn))
	{
		continue;
	}

	const USceneComponent* Root = Pawn->GetRootComponent();
	const FVector PawnLocation = Root != nullptr ? Root->GetComponentLocation() : FVector::ZeroVector;
	if (FVector::DistSquared(PawnLocation, SourcePosition) <= RangeSquared)
	{
		OutFortPawns.Add(Pawn);
	}
}
