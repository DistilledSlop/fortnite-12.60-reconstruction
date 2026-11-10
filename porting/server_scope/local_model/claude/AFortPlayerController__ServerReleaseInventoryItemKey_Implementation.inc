// CONFIDENCE: LOW. Impl at 0x1423abdb0 is fully read. FUN_142014c00, FUN_1428e9db0 and the slot 0x530/0x14d8 targets are not read.
AFortItem* Item = WorldInventory->FindItemByGuid(ItemGuid);   // this+0x598, virtual slot 0xb0
if (Item != nullptr)
{
    UObject* Def = Item->GetItemDefinition();   // virtual slot 0x3e8 (named by slot only)
    if (Def != nullptr && Def->IsA(ItemDefinitionClass)   // class from FUN_1428e9db0, unnamed && CanReleaseKey(Def))   // FUN_142014c00
    {
        Def->ReleaseItemKey(Def, Item, GetKeyContext());   // virtual slot 0x530 on Def; GetKeyContext = virtual slot 0x14d8 on this
    }
}
