// Original body 0x144f26d30 (thunk 0x14526f210). Offset evidence: HostItemManagementScreen (this+0x32c, reflected weak) resolves to a UFortItemManagementScreen whose FrontendInventoryFilter (+0x521) selects the sort-type byte list: 0-4 gives {1,4,2,0,8}, 5-7 gives {1,4,2,0}, 8 gives {1,4,2,7,6,5,0}, others none.
// CONFIDENCE: HIGH
static_assert(sizeof(UFortItemManagementScreen) > 0, "UFortItemManagementScreen is read for its filter");
TArray<EInventoryContentSortType> Result;
const uint8 FilterMode = static_cast<uint8>(HostItemManagementScreen.Get()->FrontendInventoryFilter);
if (FilterMode <= 4)
{
    Result.Add(static_cast<EInventoryContentSortType>(1));
    Result.Add(static_cast<EInventoryContentSortType>(4));
    Result.Add(static_cast<EInventoryContentSortType>(2));
    Result.Add(static_cast<EInventoryContentSortType>(0));
    Result.Add(static_cast<EInventoryContentSortType>(8));
}
else if (FilterMode <= 7)
{
    Result.Add(static_cast<EInventoryContentSortType>(1));
    Result.Add(static_cast<EInventoryContentSortType>(4));
    Result.Add(static_cast<EInventoryContentSortType>(2));
    Result.Add(static_cast<EInventoryContentSortType>(0));
}
else if (FilterMode == 8)
{
    Result.Add(static_cast<EInventoryContentSortType>(1));
    Result.Add(static_cast<EInventoryContentSortType>(4));
    Result.Add(static_cast<EInventoryContentSortType>(2));
    Result.Add(static_cast<EInventoryContentSortType>(7));
    Result.Add(static_cast<EInventoryContentSortType>(6));
    Result.Add(static_cast<EInventoryContentSortType>(5));
    Result.Add(static_cast<EInventoryContentSortType>(0));
}
return Result;
