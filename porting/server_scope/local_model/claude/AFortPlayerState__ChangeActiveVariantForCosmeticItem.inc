// CONFIDENCE: LOW (decompile is stack-noised; branch structure below follows the native body 0x14240c7c0 (captured, 471 lines); engine containers are named, not re-implemented)
// Original: exec thunk 0x142a88690 forwards its four reads (ItemTemplateToChange, VariantChannelToChange, DesiredActiveVariant, PreviewPawn)
// to virtual slot 0x750, which resolves to the real body 0x14240c7c0. The native body does not read PreviewPawn.
// Names: LoadoutComponent = this+0xe0; PawnRef = this+0x278; settings singleton = FUN_141ed1800 (special channel pair at +0x1978/+0x197c).
// Log verbosity gate is DAT_1481348e0 >= 3 for all three messages below.
//
// Step 1: loadout gate. Find the loadout entry for the current loadout key; if it exists and does not allow change, do nothing.
UObject* LoadoutEntry = nullptr;
if (LoadoutComponent != nullptr)
{
    LoadoutEntry = FindLoadoutEntryForCurrentKey(LoadoutComponent);   // FUN_142ec9900(LoadoutComponent, FUN_1428a71a0())
    if (LoadoutEntry != nullptr && !LoadoutEntry->IsChangeAllowed())  // vfunc 0x10
    {
        return;
    }
}
if (PawnRef == nullptr && LoadoutEntry == nullptr)
{
    return;
}

// Step 2: build the snapshot of the active cosmetic set, then resolve the cosmetic that owns ItemTemplateToChange.
// With a loadout entry the snapshot comes from it (vfunc 0x18); otherwise the pawn-side owner search is used (FUN_142323510).
FCosmeticSnapshot Snapshot;
if (PawnRef != nullptr && IsPawnClassMatch(PawnRef))              // FUN_142a75f10 class check on PawnRef
{
    Snapshot = (LoadoutEntry != nullptr) ? LoadoutEntry->GetSnapshot() : FindPawnCosmeticSnapshot(PawnRef, true);
}
else if (LoadoutEntry != nullptr)
{
    Snapshot = LoadoutEntry->GetSnapshot();
}
UObject* CosmeticItem = FindCosmeticItemForTemplate(Snapshot, ItemTemplateToChange);   // FUN_141d42070
if (CosmeticItem == nullptr)
{
    LogIfVerbose(3, "Calling ChangeActiveVariantForCosmeticItem on item template %s but the player is not using that cosmetic item", ItemTemplateToChange);
    return;
}

// Step 3: the channel object must exist and have a valid index (vfunc 0x280 returns -1 when missing).
UObject* ChannelObject = FindVariantChannel(CosmeticItem, VariantChannelToChange);   // FUN_141d507d0
if (ChannelObject == nullptr || ChannelObject->GetVariantIndex() == -1)
{
    LogIfVerbose(3, "Calling ChangeActiveVariantForCosmeticItem on %s looking for Channel %s but that item does not have that variant channel", ItemTemplateToChange, VariantChannelToChange);
    return;
}

// Step 4: find the (item, channel) entry in the active variant array (0x48-byte entries; compare template and channel names).
FVariantEntry* Entry = FindActiveVariantEntry(ActiveVariants, CosmeticItem, ItemTemplateToChange, VariantChannelToChange);
if (Entry != nullptr)
{
    if (Entry->Variant == DesiredActiveVariant)
    {
        return;   // already set: no change (native jumps to cleanup)
    }
    Entry->Variant = DesiredActiveVariant;   // update in place (+0x10 gets the desired value)
}
else
{
    // No entry for this channel yet: append one built from the desired variant (FUN_143e8b3b0 + FGameplayTagContainer_CopyAssign).
    FVariantEntry NewEntry;
    NewEntry.Template = ItemTemplateToChange;
    NewEntry.Owner = CosmeticItem;
    NewEntry.Channel = VariantChannelToChange;
    NewEntry.Variant = DesiredActiveVariant;
    ActiveVariants.Add(NewEntry);            // FUN_1407f1e50 grows storage when full
}

// Step 5: apply the updated snapshot to the pawn (native 0x14234e0f0 with PawnRef, when PawnRef is set; LoadoutComponent is not needed here).
if (PawnRef != nullptr)
{
    ApplyCosmeticSnapshotToPawn(PawnRef, Snapshot);   // FUN_14234e0f0(PawnRef, &snapshot)
}

// Step 6: if the channel name pair matches the special cosmetic channel (settings +0x1978 / +0x197c), commit to the loadout entry and refresh the pawn.
if (ChannelMatchesSpecialCosmeticChannel(ActiveVariants, ItemTemplateToChange, VariantChannelToChange))
{
    if (LoadoutEntry != nullptr)
    {
        LoadoutEntry->CommitSnapshot(Snapshot);       // vfunc 0x20 (native passes &snapshot)
    }
    RefreshPawnCosmetics(this, PawnRef);              // FUN_142407c60
}
// Otherwise: cleanup only (native LAB_14240d057 / LAB_14240d167).
