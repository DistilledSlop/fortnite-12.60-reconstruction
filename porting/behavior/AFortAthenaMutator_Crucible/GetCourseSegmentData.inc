// CONFIDENCE: high; recovered native body and reflected field mapping.
if (!CourseData.SegmentsData.IsValidIndex(SegmentId))
{
    return false;
}
OutSegmentData = CourseData.SegmentsData[SegmentId];
return true;
