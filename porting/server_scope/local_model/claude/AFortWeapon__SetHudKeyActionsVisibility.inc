// CONFIDENCE: HIGH - bounded per-index visibility write with change detection and one broadcast are exact (0x1426b3070); struct and member names are MEDIUM.
// Original addresses: exec 0x142b48660 -> native 0x1426b3070.
bool bChanged = false;
for (const FIndexVisibility& Entry : IndexVisibilityArray)
{
    if (HudKeyActionData.IsValidIndex(Entry.Index) && HudKeyActionData[Entry.Index].bIsVisible != Entry.bVisible)
    {
        HudKeyActionData[Entry.Index].bIsVisible = Entry.bVisible;
        bChanged = true;
    }
}
if (bChanged)
{
    OnWeaponHudKeyActionUpdated.Broadcast(this);
}
