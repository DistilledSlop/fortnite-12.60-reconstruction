// UNCOUNTED / UNCOMPILED DRAFT. Lift of native 0x142070440 == AFortQuickBars::SwapItems (569 lines; name proven by its log strings
// "Bad call to AFortQuickBars::SwapItems ..."). `this` = AFortQuickBars (param_1). Signature (native order):
//   bool AFortQuickBars::SwapItems(uint8 BarA, int32 SlotA, uint8 BarB, int32 SlotB)   // param_2..param_5
// Full native disassembly proves early failure returns zero at 0x142070ef0, fit failures clear BL, and success sets BL at 0x142070e25.
// Reflected boolean setters prove slot masks: bEnabled=1, bIsDirty=2, bIsReserved=4, bIsOccupied=8 at native +0x10.
// Quickbar offsets: Primary 0x220 / Secondary 0x2b0 / Creative 0x340 (reflected). FQuickBar: CurrentFocusedSlot +0, Slots +0x10 (stride 0x28),
// DataDefinition +0x20 (QuickbarSlots, stride 0x40). FQuickBarSlot: Items +0 (TArray<FGuid>), flags +0x10, UsedBySlotIndex +0x14, UsedByItemGuid +0x18.
using namespace InventoryLarge101Native;

const auto PickBar = [this](uint8 Bar) -> FQuickBar&               // 0->0x220, 1->0x2b0, 2->0x340 (callers guarantee <=2)
{
    return (Bar == 1) ? SecondaryQuickBar : (Bar == 2) ? CreativeQuickBar : PrimaryQuickBar;
};
const auto IsZeroGuid = [](const FGuid& G) { return G.A == 0 && G.B == 0 && G.C == 0 && G.D == 0; };

// 1. Inherited AActor::Owner at native +0xe0 must be an AFortPlayerController.
UObject* OwnerObj = GetOwner();
AFortPlayerController* PC = (OwnerObj && OwnerObj->IsA(AFortPlayerController::StaticClass())) ? (AFortPlayerController*)OwnerObj : nullptr;
if (BarA > 2 || BarB > 2 || !PC)
    return false;                                                   // silent

FQuickBar& QA = PickBar(BarA);
FQuickBar& QB = PickBar(BarB);
const auto InSlots = [](FQuickBar& Q, int32 I) { return I >= 0 && I < Q.Slots.Num(); };
const auto InData  = [](FQuickBar& Q, int32 I) { return I >= 0 && I < Q.DataDefinition.QuickbarSlots.Num(); };
const auto LogBad = [&](const TCHAR* Fmt, int32 SA, int32 SB, int32 N1, int32 N2)
{
    // Native category constructor 0x1402cf420 names 0x1481348e0 LogFort; native verbosity 2 is Error.
    if (!LogFort.IsSuppressed(ELogVerbosity::Error))
        FMsg::Logf_Internal(nullptr, 0, LogFort.GetCategoryName(), ELogVerbosity::Error, Fmt, SA, SB, N1, N2);
};

bool bOk = false;                                                   // BL
do
{
    // 2. index/flag validation (any failure -> "Invalid index" log at the tail)
    if (InSlots(QA, SlotA) && QA.Slots[SlotA].bEnabled && InData(QA, SlotA)
        && InSlots(QB, SlotB) && QB.Slots[SlotB].bEnabled && InData(QB, SlotB))
    {
        // 3. occupant guids: Items[0] if any; else UsedByItemGuid when flag&8; else zero
        const auto ReadGuid = [&](FQuickBarSlot& S) -> FGuid
        {
            if (S.Items.Num() >= 1) return S.Items[0];
            if (S.bIsOccupied)   return S.UsedByItemGuid;
            return FGuid();
        };
        const FGuid GA = ReadGuid(QA.Slots[SlotA]);
        const FGuid GB = ReadGuid(QB.Slots[SlotB]);
        const bool bHasA = !IsZeroGuid(GA);
        const bool bHasB = !IsZeroGuid(GB);
        // PC+0x598 object vtbl +0xb0(&Guid) -> UFortItem*
        UFortItem* ItemA = bHasA ? PC->NativeFindItemByGuid(GA) : nullptr;
        UFortItem* ItemB = bHasB ? PC->NativeFindItemByGuid(GB) : nullptr;
        // item vtbl +0x2e8 -> uint8 slot count (default 1 when no item)
        const uint8 SizeA = ItemA ? ItemA->GetNumberOfSlotsTaken() : 1;
        const uint8 SizeB = ItemB ? ItemB->GetNumberOfSlotsTaken() : 1;
        if (BarA != BarB && SizeA != SizeB && (SizeA != 0 || SizeB != 0))
        {
            LogBad(TEXT("Bad call to AFortQuickBars::SwapItems with index %d %d and item size %d %d (Invalid item size)!"), SlotA, SlotB, SizeA, SizeB);
            break;
        }
        // 4. head slots (flag&8 -> UsedBySlotIndex) then size-difference shift
        int32 HeadA = QA.Slots[SlotA].bIsOccupied ? QA.Slots[SlotA].UsedBySlotIndex : SlotA;
        int32 HeadB = QB.Slots[SlotB].bIsOccupied ? QB.Slots[SlotB].UsedBySlotIndex : SlotB;
        if (HeadB < HeadA) HeadA += (int32)SizeA - (int32)SizeB;
        else               HeadB += (int32)SizeB - (int32)SizeA;
        if (!(InSlots(QA, HeadA) && QA.Slots[HeadA].bEnabled && InData(QA, HeadA)
              && InSlots(QB, HeadB) && QB.Slots[HeadB].bEnabled && InData(QB, HeadB)))
        {
            LogBad(TEXT("Bad call to AFortQuickBars::SwapItems with index %d %d (Invalid index)!"), SlotA, SlotB, 0, 0);
            break;
        }
        // 5. accepted item types (silent return on mismatch). B's item goes to A's head slot; A's item to B's head slot.
        const auto Accepts = [&](FQuickBar& Q, int32 I, UFortItem* It)
        {
            const uint8 T = (uint8)It->NativeGetItemType();                 // vtbl +0x318
            for (const auto E : Q.DataDefinition.QuickbarSlots[I].AcceptedItemTypes) if ((uint8)E == T) return true;
            return false;
        };
        if (ItemB && !Accepts(QA, HeadA, ItemB)) break;
        if (ItemA && !Accepts(QB, HeadB, ItemA)) break;

        // 6. The shared quickbar draft defines FQuickbarMovePair and FQuickbarPending.
        // Pending values are {Item@+0x10, FromSlot@+0x18, ToSlot@+0x1c}.
        TArray<FGuid> MoveGuids; MoveGuids.Add(GA); MoveGuids.Add(GB);
        FQuickbarPending Pending;                                       // native sparse bit-array iteration; stride 0x28
        bool bFits = false;
        if (BarA == BarB)
        {
            TArray<FQuickbarMovePair> Moves;                              // 16-byte {UFortItem*, int32 slot}
            if (ItemA) Moves.Add({ItemA, HeadB});
            if (ItemB) Moves.Add({ItemB, HeadA});
            const bool bPush = (HeadB < HeadA) ? (SizeA <= SizeB) : (SizeB < SizeA);
            bFits = Native_14204f2a0(this, Moves, QA, bPush, MoveGuids, Pending);     // fills Pending
            if (!bFits)
                LogBad(TEXT("Bad call to AFortQuickBars::SwapItems with index %d %d and item size %d %d (Cannot fit item with pushing)!"), SlotA, SlotB, SizeA, SizeB);
        }
        else
        {
            if (ItemA && !Native_14204e850(this, ItemA, QB, HeadB, MoveGuids, /*bIgnore*/ 0))
                LogBad(TEXT("Bad call to AFortQuickBars::SwapItems with index %d %d and item size %d %d (Cannot fit item A)!"), SlotA, SlotB, SizeA, SizeB);
            else if (ItemB && !Native_14204e850(this, ItemB, QA, HeadA, MoveGuids, /*bIgnore*/ 0))
                LogBad(TEXT("Bad call to AFortQuickBars::SwapItems with index %d %d and item size %d %d (Cannot fit item B)!"), SlotA, SlotB, SizeA, SizeB);
            else bFits = true;
        }
        if (!bFits) break;                                                // BL = 0

        // 7. COMMIT (LAB_142070aba)
        if (bHasA) Native_142065b00(QA, GA, SlotA);                       // clear occupancy by guid
        if (bHasB) Native_142065b00(QB, GB, SlotB);
        for (const auto& M : Pending)                      // bit-array order
        {
            Native_142065b00(QA, M.Key, M.Value.FromSlot);
            AssignItemToSlot(QA, M.Value.Item, M.Value.ToSlot);                      // 142049220
        }
        if (bHasA)
        {
            AssignItemToSlot(QB, ItemA, HeadB);
            Native_1417cdda0(PC, ItemA, BarB, SlotA, SlotB, ItemB);       // callback; 6 args per asm (rcx=PC)
        }
        if (bHasB)
        {
            AssignItemToSlot(QA, ItemB, HeadA);
            Native_1417cdda0(PC, ItemB, BarA, SlotB, SlotA, ItemA);
        }
        if (BarA == BarB)
        {
            uint8 FocusBar; int32 FocusSlot = HeadA; bool bBroadcast = true;
            if (QA.CurrentFocusedSlot == SlotA) { QA.CurrentFocusedSlot = HeadB; FocusBar = BarA; FocusSlot = HeadB; }
            else if (QB.CurrentFocusedSlot == SlotB) { QB.CurrentFocusedSlot = HeadA; FocusBar = BarB; FocusSlot = HeadA; }
            else bBroadcast = false;
            if (bBroadcast) Native_14204dd50_Broadcast((uint8*)this + 0x400, FocusBar, FocusSlot);   // multicast delegate (bar, slot), then CompactInvocationList
        }
        Native_14205c5e0(this, BarA);
        if (BarA != BarB) Native_14205c5e0(this, BarB);
        bOk = true;                                                       // BL=1 at 0x142070e25
        break;
    }
    LogBad(TEXT("Bad call to AFortQuickBars::SwapItems with index %d %d (Invalid index)!"), SlotA, SlotB, 0, 0);
} while (false);
// scope exit: Pending set, MoveGuids, Moves arrays freed (FMemory_Free); working containers reset.
return bOk;

// ---- 142049220 AssignItemToSlot (lifted; body proven) ----
// void AssignItemToSlot(FQuickBar& Q, UFortItem* Item, int32 Slot):
//   if (!Item || Slot<0 || Slot>=Q.Slots.Num() || !Q.Slots[Slot].bEnabled) return;
//   FGuid G = Item->NativeGetItemGuid(); if (G is zero) return;      // vtbl +0x468
//   FQuickBarSlot& S = Q.Slots[Slot];
//   if (S.bEnabled && G nonzero) { if (S.Items.Num()==0) S.Items.SetNumUninitialized(1) (grow via 140690dd0); S.Items[0]=G; S.bIsDirty=true; }
//   uint8 N = Item->vtbl+0x2e8();
//   for (i=Slot+1; i < Slot+N; ++i) if (InRange && Q.Slots[i].bEnabled) { Slot_i.UsedByItemGuid=G; Slot_i.bIsDirty=true; Slot_i.bIsOccupied=true; Slot_i.UsedBySlotIndex=Slot; }
