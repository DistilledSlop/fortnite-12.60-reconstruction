// Original body address: 0x142083780
// Offset evidence: TeamArray Array at +0x7c0 (count +0x7c8), element stride 0x28; element MaxInitTeamSize at +0x1c, InitTeamSizeWeight at +0x20, bHasBucketAvailable byte at +0x24. Empty array returns true.
// CONFIDENCE: HIGH
int32 MinA = 0x7fffffff;
int32 MaxA = static_cast<int32>(0x80000000u);
int32 MinB = 0x7fffffff;
int32 MaxB = static_cast<int32>(0x80000000u);
for (int32 i = 0; i < TeamArray.Num(); ++i)
{
	const FMinigameTeam& Team = TeamArray[i];
	if (!Team.bHasBucketAvailable) { continue; }
	const int32 A = Team.MaxInitTeamSize;
	const int32 B = Team.InitTeamSizeWeight;
	MinA = (MinA <= A) ? MinA : A;
	MaxA = (MaxA >= A) ? MaxA : A;
	MinB = (MinB <= B) ? MinB : B;
	MaxB = (MaxB >= B) ? MaxB : B;
}
return (MinA != MaxA) || (MinB != MaxB);
