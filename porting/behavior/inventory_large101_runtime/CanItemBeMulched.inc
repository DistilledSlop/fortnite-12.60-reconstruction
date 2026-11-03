// UNCOUNTED / UNCOMPILED. Lift of native helper 0x144cfad30 (wrapper 0x14525e140).
// bool UFortInventoryContext::CanItemBeMulched(UFortItem* Item, FText& OutRestrictionReasons) const
// NOT closed: 0x144cfb480 (reason producer), item vtbl +0x4f0 for UFortAccountItem, and 140822780/142ee3b30/141656200 bodies are missing.
using namespace InventoryLarge101Native;
TArray<FText> Reasons;
bool bResult = false;
if (Item)
{
    if (!Item->IsA(UFortWorldItem::StaticClass()))                       // getter 0x142b5bd80
    {
        TArray<uint8> Codes;
        bResult = Native_144cfb480(this, Item, Codes);
        for (const uint8 Code : Codes)
        {
            // Literals read from image-125 (UTF-16): NSLOCTEXT(Namespace, Key, Text). NB: key/text pairs are exactly as in the binary.
            switch (Code)
            {
            case 0: Reasons.Add(NSLOCTEXT("ItemRecyclingRestrictionReasons", "InnatelyUnrecyclable", "This item is permanent.")); break;
            case 1: Reasons.Add(NSLOCTEXT("ItemRecyclingRestrictionReasons", "IsSlottedGroundOperative", "Cannot retire while assigned to a Hero Loadout.")); break;
            case 2: Reasons.Add(NSLOCTEXT("ItemRecyclingRestrictionReasons", "MissingCatalyst", "Required nodes in the skill tree have not been purchased.")); break;
            case 3: Reasons.Add(NSLOCTEXT("ItemRecyclingRestrictionReasons", "ItemOutOnExpedition", "Cannot retire while on an Expedition.")); break;
            case 4: Reasons.Add(NSLOCTEXT("ItemRecyclingRestrictionReasons", "InUseByCrafting", "Item is in use by crafting.")); break;
            case 5: Reasons.Add(NSLOCTEXT("ItemRecyclingRestrictionReasons", "ModeDoesNotAllowRecycling", "The game mode you are playing does not allow recycling.")); break;
            case 6: Reasons.Add(NSLOCTEXT("ItemRecyclingRestrictionReasons", "IsSlottedAttributeWorker", "Cannot retire while assigned to a Survivor Squad.")); break;
            default: Reasons.Add(FText::GetEmpty()); break;               // 1407c2c80 == FText::GetEmpty() (global empty FText, refcount copy)
            }
        }
        if (Item->IsA(UFortAccountItem::StaticClass()) && bResult)        // getter 0x14278d210
        {
            if (Item_Slot0x4f0(Item))
            {
                Reasons.Add(NSLOCTEXT("InventoryContextMulchRestrictionReasons", "IsFavorited", "This item is favorited."));
                bResult = false;
            }
            if (Item->IsA(UFortHero::StaticClass()))                      // getter 0x142939040
            {
                // 144cf0e90: typed-outer lookup of UFortHomebaseUIContext (getter 0x1452518e0) + IsA re-check, else null. Result is NOT null-checked by caller.
                UObject* Home = Native_140822780(this, UFortHomebaseUIContext::StaticClass());
                if (Home && !Home->IsA(UFortHomebaseUIContext::StaticClass())) Home = nullptr;
                // 1450b2c80: copy NotThreadSafe shared ptr at Home+0xc8 (obj) / +0xd0 (controller, strong count at ctrl+8).
                uint8* SharedObj = *(uint8**)((uint8*)Home + 0xc8);
                // 14129b8c0(SharedObj) == SharedObj + 0x158 : map used below.
                const void* HeroMap = SharedObj + 0x158;
                // 14511b280(HeroMap, Out, Item, /*param_4*/0): param_4==0 skips the overlay map. Key = Native_142ee3b30(Item) ({-1,..} if Item null).
                int32 Key[2] = {-1, 0};
                if (Item) Native_142ee3b30(Key, Item);
                int32 Index = -1;
                Native_141656200(HeroMap, &Index, Key);
                // entry = *(uint8**)HeroMap + Index*0x1c ; value at entry+8: {FName(8) ; int32}. Absent -> {None(0,0), -1}.
                int32 SlotName[2] = {0, 0};
                if (Index != -1) { const int32* V = (const int32*)(*(uint8**)HeroMap + (int64)Index * 0x1c + 8); SlotName[0] = V[0]; SlotName[1] = V[1]; }
                if (SlotName[1] != 0 || !FName_EqualsEName(SlotName[0], 0))   // name != None
                {
                    Reasons.Add(NSLOCTEXT("InventoryContextMulchRestrictionReasons", "IsSlottedHero", "This hero is equipped."));
                    bResult = false;
                }
                // raw this+0x204 / this+0x318 (int32, not reflected; owning field names unproven)
                if (*(const int32*)((const uint8*)this + 0x204) <= *(const int32*)((const uint8*)this + 0x318) + 1)
                {
                    Reasons.Add(NSLOCTEXT("InventoryContextMulchRestrictionReasons", "IsLastHero", "This is your last hero."));
                    bResult = false;
                }
                // shared ptr released non-atomically here.
            }
        }
    }
    else
    {
        TArray<uint8> Codes;
        // Item vtbl +0x448: WorldItem/CreativeWorldItem/PlaysetWorldItem all -> 0x142074560 == movzx eax, byte [this+0x145]; ret.
        // 0x145 lies in FortItemEntry (ItemEntry @0xe0 -> +0x65, inside the bool byte(s) after ControlOverride@0x60).
        if (*((const uint8*)Item + 0x145) != 0)
        {
            Codes.Add(1);
        }
        else
        {
            UFortItemDefinition* Def = Item->GetItemDefinitionBP();      // vtbl +0x3e8 (WorldItem target 0x14205a2f0 forwards ItemEntry @+0xe0)
            bool bOk = false;
            if (Def && Def->IsA(UFortWorldItemDefinition::StaticClass())) // getter 0x142b5be40
            {
                // 14204e530: DisassembleRecipe (FDataTableRowHandle @ def+0x710) valid and its row resolves.
                UFortWorldItemDefinition* WDef = static_cast<UFortWorldItemDefinition*>(Def);
                const FDataTableRowHandle& H = WDef->DisassembleRecipe;
                if (H.DataTable != nullptr || H.RowName != NAME_None)
                {
                    const FString Context = FString::Printf(TEXT("DisassembleRecipe for %s"), *WDef->GetFName().ToString());
                    bOk = Native_141ff8510(H, Context) != nullptr;
                }
            }
            if (bOk) bResult = true; else Codes.Add(0);
        }
        for (const uint8 Code : Codes)
        {
            if (Code == 0)      Reasons.Add(NSLOCTEXT("ItemDisassembleRestrictionReasons", "InnatelyCannotDisassemble", "This item does not disassemble."));
            else if (Code == 1) Reasons.Add(NSLOCTEXT("ItemDisassembleRestrictionReasons", "ItemWasGifted", "This item was gifted and cannot be disassembled."));
            else                Reasons.Add(FText::GetEmpty());
        }
    }
}
// 142c40560: FText::Join(delimiter "\r\n" read from image 0x1459ac134, Reasons); Num==1 copies element 0. Result swapped into the out param.
OutRestrictionReasons = (Reasons.Num() == 1) ? Reasons[0] : FText::Join(FText::FromString(TEXT("\r\n")), Reasons);
return bResult;
