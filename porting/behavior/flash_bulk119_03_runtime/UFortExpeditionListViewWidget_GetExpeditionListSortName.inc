// CONFIDENCE: high
// Native helper 0x1450e6340.
switch (SortType)
{
case EFortExpeditionListSort::ByRating:
    return NSLOCTEXT("ExpeditionListSortNamespace", "ByRatingSortType", "By Rating");
case EFortExpeditionListSort::ByDuration:
    return NSLOCTEXT("ExpeditionListSortNamespace", "ByDurationSortType", "By Duration");
case EFortExpeditionListSort::ByName:
    return NSLOCTEXT("ExpeditionListSortNamespace", "ByNameSortType", "By Name");
default:
    return FText::GetEmpty();
}
