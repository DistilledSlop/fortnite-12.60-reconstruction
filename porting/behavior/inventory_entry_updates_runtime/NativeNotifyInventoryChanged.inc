// CONFIDENCE: HIGH
// 0x14201de90. Mark the array before publishing either local flag.
Inventory.MarkArrayDirty();
bRequiresLocalUpdate = true;
bRequiresSaving = true;
ForceNetUpdate();
