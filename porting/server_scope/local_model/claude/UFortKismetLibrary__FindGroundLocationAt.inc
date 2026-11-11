// CONFIDENCE: MEDIUM
// Native 0x14296af10 -> 0x141eede00: the default result is InLocation. Only when the world's game state is an
// AFortGameStateAthena with a MapInfo does a single profile line trace run down the column at InLocation (from
// TraceStartZ to TraceEndZ), ignoring IgnoreActor. The impact point is returned on hit, otherwise the trace end.
FVector Result = InLocation;
const AFortGameStateAthena* GameStateAthena = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
if (!GameStateAthena || !GameStateAthena->MapInfo)
{
    return Result;
}
const FVector Start(InLocation.X, InLocation.Y, TraceStartZ);
const FVector End(InLocation.X, InLocation.Y, TraceEndZ);
FCollisionQueryParams QueryParams(FName(TEXT("FindGroundLocationAt")), false);
QueryParams.AddIgnoredActor(IgnoreActor);
FHitResult Hit;
if (World->LineTraceSingleByProfile(Hit, Start, End, TraceName, QueryParams))
{
    Result = Hit.Location;
}
else
{
    Result = End;
}
return Result;
