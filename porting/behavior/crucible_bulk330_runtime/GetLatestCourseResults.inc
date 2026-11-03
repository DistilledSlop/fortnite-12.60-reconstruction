// CONFIDENCE: high; captured native branches and reflected fields.
OutIsInProgress = !(LatestCourseResults.StartTime < 0.0f || LatestCourseResults.FinishTime >= 0.0f || LatestCourseResults.CancelTime >= 0.0f);
OutHasBeenCanceled = LatestCourseResults.StartTime >= 0.0f && LatestCourseResults.CancelTime >= 0.0f;
return LatestCourseResults;
