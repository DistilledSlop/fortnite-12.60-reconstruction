// CONFIDENCE: high
// Native 0x14208efc0; wrapper 0x1429bfd70.
{
    WinCount = 0;
    if (!PlayerBuckets.IsValidIndex(BucketIndex))
    {
        return false;
    }
    for (int32 Winner : RoundWinHistory)
    {
        if (Winner == BucketIndex)
        {
            ++WinCount;
        }
    }
    return true;
}
