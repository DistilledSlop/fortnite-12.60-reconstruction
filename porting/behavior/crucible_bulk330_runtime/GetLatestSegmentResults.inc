// CONFIDENCE: high; captured native branches and reflected fields.
for (int32 Index = LatestCourseResults.SegmentResults.Num() - 1; Index >= 0; --Index)
{
    const FCrucibleSegmentResults& Candidate = LatestCourseResults.SegmentResults[Index];
    if (Candidate.SegmentId != SegmentId) continue;
    OutSegmentResults = Candidate;
    OutIsInProgress = !(OutSegmentResults.StartTime < 0.0f || OutSegmentResults.FinishTime >= 0.0f || OutSegmentResults.CancelTime >= 0.0f);
    OutHasBeenCanceled = OutSegmentResults.StartTime >= 0.0f && OutSegmentResults.CancelTime >= 0.0f;
    return true;
}
OutSegmentResults.SegmentId = -1;
OutSegmentResults.CalculatedScore = -1.0f;
OutSegmentResults.CalculatedPenalty = -1.0f;
OutSegmentResults.CalculatedMissedTargets = 0;
OutSegmentResults.StartTime = -1.0f;
OutSegmentResults.FinishTime = -1.0f;
OutSegmentResults.CancelTime = -1.0f;
OutSegmentResults.NumAIElims = 0;
OutSegmentResults.NumTargetElims = 0;
OutIsInProgress = false;
OutHasBeenCanceled = false;
return false;
