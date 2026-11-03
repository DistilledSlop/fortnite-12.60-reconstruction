// UNCOUNTED DRAFT - 0x142070440 (569-line decompile; x86 re-read from image-125.runtime.bin 0x142070440..0x142070f00 for dropped args).
// NOTE: this address is NOT CheckGhostModeItemReplicated (see CheckGhostModeItemReplicated.inc / README): its own log strings name
// "AFortQuickBars::SwapItems"; CheckGhostModeItemReplicated merely calls it. Signature: params (this, byte, int, byte, int) = (BarA, IndexA, BarB, IndexB).
// void AFortQuickBars::SwapItems(EFortQuickBars BarA, int32 IndexA, EFortQuickBars BarB, int32 IndexB)
#include "Unresolved97.h"

static_assert(sizeof(FQuickBar) == 0x90, "bars are 0x90 apart (0x220/0x2b0/0x340)");
static_assert(sizeof(FQuickBarSlot) == 0x28, "slot stride 0x28");
static_assert(sizeof(FQuickBarSlotData) == 0x40, "slot-data stride 0x40");
static_assert(offsetof(FQuickBar, Slots) == 0x10 && offsetof(FQuickBar, DataDefinition) == 0x20, "native reads Slots at +0x10/+0x18 and data defs at +0x20/+0x28");

namespace
{
	// native 0x220/0x2b0/0x340 selection; any other value falls to 0x220 (never reached: >2 returns earlier).
	FQuickBar* Swap_BarPtr(AFortQuickBars* QB, uint8 Bar)
	{
		if (Bar == 1) return &QB->SecondaryQuickBar;
		if (Bar == 2) return &QB->CreativeQuickBar;
		return &QB->PrimaryQuickBar;
	}
	FORCEINLINE bool Swap_SlotIdxValid(const FQuickBar& B, int32 I) { return I >= 0 && I < B.Slots.Num(); }
	FORCEINLINE bool Swap_DataIdxValid(const FQuickBar& B, int32 I) { return I >= 0 && I < B.DataDefinition.QuickbarSlots.Num(); }
	// guid occupying a slot: Items[0] if any, else UsedByItemGuid when bIsOccupied, else zero guid.
	FGuid Swap_SlotGuid(const FQuickBarSlot& S)
	{
		if (S.Items.Num() >= 1) return S.Items[0];
		return S.bIsOccupied ? S.UsedByItemGuid : FGuid(0, 0, 0, 0);
	}
	FORCEINLINE bool Swap_GuidNonZero(const FGuid& G) { return (G.A | G.B | G.C | G.D) != 0; }
	bool Swap_AcceptsType(const FQuickBarSlotData& D, uint8 ItemType)
	{
		for (int32 i = 0; i < D.AcceptedItemTypes.Num(); ++i) if (static_cast<uint8>(D.AcceptedItemTypes[i]) == ItemType) return true;
		return false;
	}
	FORCEINLINE void Swap_Log(const TCHAR* Fmt, int32 A, int32 B, int32 C = 0, int32 D = 0)
	{
		if (Unresolved_LogVerbosity_1481348e0 >= 2) Unresolved_LogSwapItemsError_142c54b70(Fmt, A, B, C, D);
	}
}

void AFortQuickBars::SwapItems(EFortQuickBars BarA, int32 IndexA, EFortQuickBars BarB, int32 IndexB)
{
	// Owner (raw +0xe0) cast to the class from 0x142a3d9f0; native uses the class-tree index test.
	UObject* Owner = *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(this) + 0xe0);
	UObject* Controller = (Owner && Owner->GetClass()->IsChildOf(Unresolved_StaticClass_142a3d9f0())) ? Owner : nullptr;
	if (static_cast<uint8>(BarA) > 2 || static_cast<uint8>(BarB) > 2 || !Controller) return; // silent

	FQuickBar* PA = Swap_BarPtr(this, static_cast<uint8>(BarA));
	FQuickBar* PB = Swap_BarPtr(this, static_cast<uint8>(BarB));

	// ---- raw index validation, A then B (each: Slots bounds, bEnabled, data-definition bounds). Failure => "Invalid index" log at the bottom.
	bool bIndicesOk = false;
	FQuickBarSlot* SlotA = nullptr; FQuickBarSlot* SlotB = nullptr;
	if (Swap_SlotIdxValid(*PA, IndexA))
	{
		SlotA = &PA->Slots[IndexA];
		if (SlotA->bEnabled && Swap_DataIdxValid(*PA, IndexA) && Swap_SlotIdxValid(*PB, IndexB))
		{
			SlotB = &PB->Slots[IndexB];
			bIndicesOk = SlotB->bEnabled && Swap_DataIdxValid(*PB, IndexB);
		}
	}
	if (!bIndicesOk)
	{
		Swap_Log(L"Bad call to AFortQuickBars::SwapItems with index %d %d (Invalid index)!", IndexA, IndexB);
		return;
	}

	// ---- item guids (zero guid => empty), items resolved through the inventory-owner interface at controller+0x598 (vtable +0xb0)
	const FGuid GuidA = Swap_SlotGuid(*SlotA);
	const FGuid GuidB = Swap_SlotGuid(*SlotB);
	const bool bHasA = Swap_GuidNonZero(GuidA);   // uStack_170 != 0
	const bool bHasB = Swap_GuidNonZero(GuidB);   // uStack_16c != 0
	void* Iface = reinterpret_cast<uint8*>(Controller) + 0x598;
	UObject* ItemA = bHasA ? Unresolved_InventoryOwnerInterface_FindItemByGuid_vtB0(Iface, GuidA) : nullptr;
	UObject* ItemB = bHasB ? Unresolved_InventoryOwnerInterface_FindItemByGuid_vtB0(Iface, GuidB) : nullptr;
	const uint8 SizeA = ItemA ? Unresolved_Item_GetItemSize_vt2e8(ItemA) : 1; // null item counts as size 1
	const uint8 SizeB = ItemB ? Unresolved_Item_GetItemSize_vt2e8(ItemB) : 1;

	if (BarA != BarB && SizeA != SizeB && (SizeA != 0 || SizeB != 0))
	{
		Swap_Log(L"Bad call to AFortQuickBars::SwapItems with index %d %d and item size %d %d (Invalid item size)!", IndexA, IndexB, SizeA, SizeB);
		return;
	}

	// ---- effective (root) slot indices, then shift by the size difference on the later side
	int32 EffA = SlotA->bIsOccupied ? SlotA->UsedBySlotIndex : IndexA; // iVar17
	int32 EffB = SlotB->bIsOccupied ? SlotB->UsedBySlotIndex : IndexB; // iVar18
	if (EffB < EffA) EffA += (int32)SizeA - (int32)SizeB;
	else             EffB += (int32)SizeB - (int32)SizeA;

	// recheck on the shifted indices (same three tests, A then B); native then falls to the "Invalid index" log (no cleanup needed yet)
	if (!(Swap_SlotIdxValid(*PA, EffA) && PA->Slots[EffA].bEnabled && Swap_DataIdxValid(*PA, EffA)
	   && Swap_SlotIdxValid(*PB, EffB) && PB->Slots[EffB].bEnabled && Swap_DataIdxValid(*PB, EffB)))
	{
		Swap_Log(L"Bad call to AFortQuickBars::SwapItems with index %d %d (Invalid index)!", IndexA, IndexB);
		return;
	}

	// ---- slot item-type acceptance: B's item must fit A's effective slot, then A's item must fit B's; silent return on failure
	if (ItemB && !Swap_AcceptsType(PA->DataDefinition.QuickbarSlots[EffA], Unresolved_Item_GetItemType_vt318(ItemB))) return;
	if (ItemA && !Swap_AcceptsType(PB->DataDefinition.QuickbarSlots[EffB], Unresolved_Item_GetItemType_vt318(ItemA))) return;

	// ---- fit checks. Ignored guids = {GuidA, GuidB}; pending-move set is filled only by the push helper.
	TArray<FGuid> Ignored; Ignored.Add(GuidA); Ignored.Add(GuidB);
	FRawPendingMoveSet Pending; // native stack object: TSet with 0x28 elements + 0x80-bit inline bit array
	bool bProceed = false;
	if (BarA == BarB)
	{
		TArray<FRawSlotMove> Moves;
		if (ItemA) Moves.Add({ ItemA, EffB, 0 /* native high dword = leftover guid word, never read as far as recovered */ });
		if (ItemB) Moves.Add({ ItemB, EffA, 0 });
		const bool bPushDir = (EffB < EffA) ? (SizeA <= SizeB) : (SizeB < SizeA);
		if (Unresolved_QuickBars_TryFitWithPush_14204f2a0(this, Moves, PA, bPushDir, Ignored, Pending)) bProceed = true;
		else Swap_Log(L"Bad call to AFortQuickBars::SwapItems with index %d %d and item size %d %d (Cannot fit item with pushing)!", IndexA, IndexB, SizeA, SizeB);
	}
	else if (!ItemA || Unresolved_QuickBars_CanFitItem_14204e850(this, ItemA, PB, EffB, Ignored, false))
	{
		if (!ItemB || Unresolved_QuickBars_CanFitItem_14204e850(this, ItemB, PA, EffA, Ignored, false)) bProceed = true;
		else Swap_Log(L"Bad call to AFortQuickBars::SwapItems with index %d %d and item size %d %d (Cannot fit item B)!", IndexA, IndexB, SizeA, SizeB);
	}
	else Swap_Log(L"Bad call to AFortQuickBars::SwapItems with index %d %d and item size %d %d (Cannot fit item A)!", IndexA, IndexB, SizeA, SizeB);

	if (!bProceed) return; // native frees Ignored/Pending temporaries; RAII here

	// ---- apply: remove both, drain pushed moves into bar A, place swapped items, notify, fix focus, refresh
	if (bHasA) Unresolved_QuickBar_RemoveItem_142065b00(PA, GuidA, IndexA);
	if (bHasB) Unresolved_QuickBar_RemoveItem_142065b00(PB, GuidB, IndexB);
	// Drain loop over Pending (bit-array order, lowest set bit first): for each entry
	//   RemoveItem(PA, &entry.ItemGuid, entry.OldSlot); PlaceItem(PA, entry.Item, entry.NewSlot);
	// iteration/removal-from-bit-array internals (0x140687d00 etc.) are unresolved; see README.
	Unresolved_DrainPendingMoves_Bar(PA, Pending);
	if (bHasA) { Unresolved_QuickBar_PlaceItem_142049220(PB, ItemA, EffB); Unresolved_QuickBars_NotifyItemMoved_1417cdda0(Controller, ItemA, static_cast<uint8>(BarB), IndexA, IndexB, ItemB); }
	if (bHasB) { Unresolved_QuickBar_PlaceItem_142049220(PA, ItemB, EffA); Unresolved_QuickBars_NotifyItemMoved_1417cdda0(Controller, ItemB, static_cast<uint8>(BarA), IndexB, IndexA, ItemA); }

	if (BarA == BarB)
	{
		bool bNotifyFocus = true; uint8 FocusBar = static_cast<uint8>(BarA); int32 FocusIdx = EffA;
		if (PA->CurrentFocusedSlot == IndexA) { PA->CurrentFocusedSlot = EffB; FocusIdx = EffB; }
		else if (PB->CurrentFocusedSlot == IndexB) { PB->CurrentFocusedSlot = EffA; FocusBar = static_cast<uint8>(BarB); }
		else bNotifyFocus = false; // native skips only the focus notification, then refreshes
		if (bNotifyFocus) Unresolved_QuickBars_FocusChanged_14204dd50(reinterpret_cast<uint8*>(this) + 0x400, FocusBar, FocusIdx);
	}
	Unresolved_QuickBars_RefreshBar_14205c5e0(this, static_cast<uint8>(BarA));
	if (BarA != BarB) Unresolved_QuickBars_RefreshBar_14205c5e0(this, static_cast<uint8>(BarB));
}
