// CONFIDENCE: HIGH
// Reviewed wrapper and native implementation; build and runtime pending.
UDynamicEntryBox* TargetEntryBox = EntryBox_PositiveInteractions->GetNumEntries() > 0
    ? EntryBox_PositiveInteractions : EntryBox_NegativeInteractions;
if (TargetEntryBox->GetNumEntries() > 0)
{
    return TargetEntryBox->GetAllEntries()[0];
}
return nullptr;
