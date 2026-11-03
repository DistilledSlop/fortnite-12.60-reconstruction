// UNCOUNTED / UNCOMPILED. Lift of native helper 0x144cfe010 (wrapper 0x14525ed90).
// bool UFortInventoryContext::DropItemOnQuickBar(UFortItem* Item, EFortQuickBars TargetQuickbar, int32 TargetSlot)
// NOT closed: PC+0x598 object (vtbl +0x50/+0x20), AFortQuickBars slots 0x628/0x648, 0x142070440, 0x1420570e0, PC +0x15f0,
// 0x1423bd380 effect chain and the two FName-addressed RPC calls remain declared in UnresolvedHelpers.inc.
// Return = R12B from asm (Ghidra shows void). EFortQuickBars: Primary 0, Secondary 1, Creative 2.
using namespace InventoryLarge101Native;
const uint8 Bar = (uint8)TargetQuickbar;
const int32 Slot = TargetSlot;
UObject* Outer = GetOuter();                                              // this+0x20
if (!Outer || !Outer->IsA(ULocalPlayer::StaticClass()))                   // getter 0x144786700
    return false;
ULocalPlayer* LP = static_cast<ULocalPlayer*>(Outer);
// 14447f220 body == ULocalPlayer::GetPlayerController(UWorld*) (null world -> LP+0x30; else iterate world PCs where PC->Player == LP).
// Ctx vtbl +0x150 -> 0x140822890: Outer(+0x20)-chain walk shape of UObject::GetWorld (constructor-proven table 0x146d513f8; shape-inferred).
APlayerController* PCBase = LP->GetPlayerController(GetWorld());
if (!PCBase || !PCBase->IsA(AFortPlayerController::StaticClass()))        // getter 0x142a3d9f0
    return false;
AFortPlayerController* PC = static_cast<AFortPlayerController*>(PCBase);
if (!Item)
    return false;

// 142074570(QB, Item, Bar, Slot, Flag): body-proven. Bar 1 -> SecondaryQuickBar(0x2b0), 2 -> CreativeQuickBar(0x340), else PrimaryQuickBar(0x220).
// QuickBar fields: Slots @+0x10 (stride 0x28, byte +0x10 bit0 [first bool in declaration order = bEnabled]), DataDefinition @+0x20 whose
// QuickbarSlots (stride 0x40: AcceptedItemTypes TArray<byte> @+0, bStaticSlot byte @+0x10).
const auto CanPlace = [](AFortQuickBars* QB, UFortItem* InItem, uint8 InBar, int32 InSlot, bool bFlag) -> bool
{
    FQuickBar& B = (InBar == 1) ? QB->SecondaryQuickBar : (InBar == 2) ? QB->CreativeQuickBar : QB->PrimaryQuickBar;
    if (!InItem || InSlot < 0 || InSlot >= B.Slots.Num()) return false;
    if ((*((const uint8*)&B.Slots[InSlot] + 0x10) & 1) == 0) return false;
    if (InSlot >= B.DataDefinition.QuickbarSlots.Num()) return false;
    const FQuickBarSlotData& D = B.DataDefinition.QuickbarSlots[InSlot];
    if (bFlag && *((const uint8*)&D + 0x10) != 0) return false;
    const uint8 Type = (uint8)InItem->NativeGetItemType();                // vtbl +0x318 (-> 0x1420337a0)
    for (int32 i = 0; i < D.AcceptedItemTypes.Num(); ++i) if ((uint8)D.AcceptedItemTypes[i] == Type) return true;
    return false;
};

bool bFallback = false;
AFortQuickBars* QB = Native_PC598_Slot0x50(PC);
if (!QB || !CanPlace(QB, Item, Bar, Slot, true))
{
    bFallback = true;
}
else if (!PC_Slot0x15f0(PC, Native_141ed1800() + 0x6c0))
{
    bFallback = true;
}
else
{
    // 142057010: Item must be UFortWorldItem; guid = Item vtbl +0x468 (NativeGetItemGuid, WorldItem target 0x14205a3d0 -> ItemEntry); scan bars 0,1,2.
    uint8 ExistingBar = 0; int32 ExistingSlot = -1; bool bOnBar = false;
    if (Item->IsA(UFortWorldItem::StaticClass()))
    {
        const FGuid G = Item->NativeGetItemGuid();
        for (uint8 b = 0; b != 3; ++b)
        {
            FQuickBar& B = (b == 1) ? QB->SecondaryQuickBar : (b == 2) ? QB->CreativeQuickBar : QB->PrimaryQuickBar;
            const int32 Idx = Native_1420570e0(&B, G);
            if (Idx != -1) { ExistingBar = b; ExistingSlot = Idx; bOnBar = true; break; }
        }
    }
    if (!bOnBar)
    {
        QB_Slot0x628(QB, Item, Bar, Slot, 0);
        Native_1423bd380(PC);
        return true;
    }
    if (ExistingSlot == Slot)
        return false;                                                      // only slot int compared
    UFortItem* Target = QB_Slot0x648(QB, Bar, Slot);
    if (Target)
    {
        // Item vtbl +0x3e8 on both; slot 0x390 on ItemDefinition table 0x14606bf00 -> 0x14202fe70 reads MaxStackSize (+0x168) == NativeGetMaxStackSize(ASC=nullptr).
        if (Target->GetItemDefinitionBP() == Item->GetItemDefinitionBP())
        {
            const int32 MaxStack = Target->GetItemDefinitionBP()->NativeGetMaxStackSize(nullptr);
            // vtbl +0x368: base 0x140833bf0 returns 1; WorldItem 0x14205ac70 forwards ItemEntry (this+0xe0). Stack count (name not proven).
            const int32 Count = Item_Slot0x368(Target);
            if (Count < MaxStack)
            {
                Native_142a3e840(PC, Target->NativeGetItemGuid(), Item->NativeGetItemGuid());   // (second computed first in native)
                Native_1423bd380(PC);
                return true;
            }
        }
    }
    if (!Native_142070440(QB, ExistingBar, ExistingSlot, Bar, Slot))
        return false;                                                      // no notification
    Native_1423bd380(PC);
    return true;
}

if (bFallback)
{
    // LAB_144cfe279. 1423881a0(PC) == PC->MyPlayerInfo ? *(void**)((uint8*)MyPlayerInfo + 0xf0) : null (MyPlayerInfo @0x19e8, reflected Object).
    void* Receiver = PC->MyPlayerInfo ? *(void**)((uint8*)PC->MyPlayerInfo + 0xf0) : nullptr;
    if (!Receiver || Native_PC598_Slot0x20(PC))
        return false;
    // 144d07980(this,bar,idx): same Outer->LocalPlayer->PC resolution as above, then PC vtbl +0x1a80(bar, idx) (PC_Slot0x1a80). Null on any failure.
    const auto SlotItem = [&](uint8 InBar, int32 Idx) -> UFortItem*
    {
        UObject* O = GetOuter();
        if (!O || !O->IsA(ULocalPlayer::StaticClass())) return nullptr;
        APlayerController* P = static_cast<ULocalPlayer*>(O)->GetPlayerController(GetWorld());
        if (!P || !P->IsA(AFortPlayerController::StaticClass())) return nullptr;
        return PC_Slot0x1a80(static_cast<AFortPlayerController*>(P), InBar, Idx);
    };
    UFortItem* Slots[4] = {nullptr, nullptr, nullptr, nullptr};
    for (int32 i = 1; i < 4; ++i) Slots[i] = SlotItem(0, i);
    UFortItem* Slot4 = SlotItem(1, 4);
    UFortItem* Extra;
    if (Bar == 0)
    {
        if ((uint32)(Slot - 1) > 2) return false;
        const uint8 T = (uint8)Item->NativeGetItemType();
        if (T != 0x16 && T != 0x15) return false;                          // WeaponRanged / WeaponMelee
        int32 Found = -1;
        for (int32 i = 1; i < 4; ++i) if (Slots[i] == Item) { Found = i; break; }
        if (Found >= 0)
        {
            if (Found == Slot) return false;
            Slots[Found] = Slots[Slot];
            Slots[Slot] = Item;
        }
        else Slots[Slot] = Item;
        Extra = Slot4;
    }
    else
    {
        if (Bar != 1 || Slot != 4 || (uint8)Item->NativeGetItemType() != 0x13 || Item == Slot4) return false;   // 0x13 = Trap
        Extra = Item;
    }
    TArray<FString> Strings;
    for (int32 i = 1; i < 4; ++i) Strings.Add(Slots[i] ? Item_Slot0x470(Slots[i]) : FString());
    const FString ExtraStr = Extra ? Item_Slot0x470(Extra) : FString();
    Native_1429b0bb0(Receiver, Strings, ExtraStr, nullptr);
    Native_1423bd380(PC);
    return true;                                                          // R12B=1 at 0x144cfe51a
}
return false;
