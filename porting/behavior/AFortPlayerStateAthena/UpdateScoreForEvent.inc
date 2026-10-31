// Behaviour snippet: AFortPlayerStateAthena::UpdateScoreForEvent (kind helper: a non-reflected member; body only).
// Original address: 0x141a3c2b0 (queue row 2 of porting/behavior/CALLEE_PORT_QUEUE.md).
// Decompile source: porting/generated/callee_probe/blocking/0x141a3c2b0.c (disassembly in the .asm beside it)
// Proxy score: none (queue row 2; not a portability record)
// Inferred: for each rule of Playlist->ScoringData.ScoreDataList (FAthenaScoreData, 0x68 bytes) whose ScoringEvent is
//   Event and whose NumOccurrencesForScore is positive: points = Count / NumOccurrencesForScore (truncated); when
//   NumOccurrencesPermitted is positive the points are capped at it; unit = (int32) ScoreAwarded.GetValueAtLevel(0)
//   with the default context (FScalableFloat, GameplayAbilities); the rule's result is unit * points. When the result
//   differs from PreviousPoints, PointsAddedToScore is broadcast with (unit, Event). The return value is the result of
//   the last matching rule, or 0 when no rule matches. The loop reads the rule array and its count on every pass, as
//   the decompile does.
//   QUIRK (ported as decompiled): the broadcast carries the per-unit value `unit` in NewPoints, not the new total
//   `Result` (unit * points). The decompile writes (int) of the GetValueAtLevel result at the first parameter slot and
//   the event byte at the second.
//   The rule fields are at FAthenaScoreData +0 (ScoringEvent), +0x40 (NumOccurrencesForScore), +0x44
//   (NumOccurrencesPermitted), +0x48 (ScoreAwarded); the playlist array is at +0x4e8 (data) and +0x4f0 (count) (the
//   Eighth pass table).
// Declared in generated header: no (helper: the generator declares it, see porting/behavior/README.md, "helper")
// CONFIDENCE: HIGH
int32 Result = 0;
const UFortPlaylistAthena* ScoringPlaylist = Playlist;
for (int32 Index = 0; Index < ScoringPlaylist->ScoringData.ScoreDataList.Num(); ++Index)
{
	const FAthenaScoreData* Rule = &ScoringPlaylist->ScoringData.ScoreDataList[Index];
	if (Rule->ScoringEvent != Event || Rule->NumOccurrencesForScore <= 0)
	{
		continue;
	}
	int32 Points = Count / Rule->NumOccurrencesForScore;
	if (Rule->NumOccurrencesPermitted > 0 && Points > Rule->NumOccurrencesPermitted)
	{
		Points = Rule->NumOccurrencesPermitted;
	}
	const int32 Unit = static_cast<int32>(Rule->ScoreAwarded.GetValueAtLevel(0.f));
	Result = Unit * Points;
	if (Result != PreviousPoints)
	{
		PointsAddedToScore.Broadcast(Unit, Rule->ScoringEvent);
	}
}
return Result;
