// CONFIDENCE: HIGH
{
    return NativePendingSaveRequests.Num() > 0 &&
        (NativePendingSaveRequests[0].ItemIds48.Contains(ItemId) ||
         NativePendingSaveRequests[0].ItemIds98.Contains(ItemId));
}
