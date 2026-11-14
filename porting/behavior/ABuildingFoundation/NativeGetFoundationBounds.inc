// CONFIDENCE: HIGH
// Original 0x141bdf240; backing storage and actual cvar registration are supplied together.
if (bStreamingDataBasedBounds && GNativeFoundationUseStreamingDataBounds != 0)
{
    return StreamingData.BoundingBox;
}
return StreamingBoundingBox;
