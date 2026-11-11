// CONFIDENCE: MEDIUM
// Native 0x14296c1a0 -> 0x141ef07b0: aim assist. The source pawn must have a controller. The aim origin is the controller
// view point offset by DestOffsetVec and the aim direction is the view rotation. Candidates are AI pawns when
// bTargetAIPawns is set, otherwise player pawns, limited to Range and filtered by TeamAffiliationRequirement. The
// candidate inside MaxAngleInDegree with the highest positive score (dot with the aim direction) is returned.
if (!SourcePawn)
{
    return nullptr;
}
AController* Controller = SourcePawn->GetController();
if (!Controller)
{
    return nullptr;
}
FVector ViewLocation;
FRotator ViewRotation;
Controller->GetPlayerViewPoint(ViewLocation, ViewRotation);
const FVector AimOrigin = ViewLocation + DestOffsetVec;
const FVector AimDirection = ViewRotation.Vector();
const float CosMaxAngle = FMath::Cos(FMath::DegreesToRadians(MaxAngleInDegree));
const float RangeSquared = Range * Range;
AFortPawn* BestPawn = nullptr;
float BestScore = 1.1754944e-38f;
for (TActorIterator<AFortPawn> It(SourcePawn->GetWorld()); It; ++It)
{
    AFortPawn* Candidate = *It;
    if (!IsValid(Candidate) || Candidate == SourcePawn)
    {
        continue;
    }
    if (bTargetAIPawns == Candidate->IsPlayerControlled())
    {
        continue;
    }
    const FVector ToCandidate = Candidate->GetActorLocation() - AimOrigin;
    if (ToCandidate.SizeSquared() > RangeSquared)
    {
        continue;
    }
    if (!Candidate->IsValidAimTargetForTeamAffiliation(SourcePawn, TeamAffiliationRequirement))
    {
        continue;
    }
    const FVector ToCandidateDir = ToCandidate.GetSafeNormal();
    const float Dot = FVector::DotProduct(AimDirection, ToCandidateDir);
    if (Dot < CosMaxAngle || Dot <= BestScore)
    {
        continue;
    }
    BestScore = Dot;
    BestPawn = Candidate;
}
return BestPawn;
