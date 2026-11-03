// UNCOUNTED DRAFT - AFortPlayerControllerGameplay::CheckGhostModeItemReplicated.
// Real body = 0x1423cbe80 (wrapper 0x142a64930 only advances FFrame::Code and JMPs there; broad/impl decompiles merge stub+body and lose
// the entry prologue, so this source was read from the 0x1423cbe80..0x1423cc21e instruction stream in image-125.runtime.bin + the 161-line
// merged decompile native96-impl/0x142a64930.c). No parameters, no return. The user-assigned 0x142070440 is its callee (SwapItems).
// void AFortPlayerControllerGameplay::CheckGhostModeItemReplicated()
#include "Unresolved97.h"

void AFortPlayerControllerGameplay::CheckGhostModeItemReplicated()
{
	if (!GhostModeRepData.bInGhostMode) return; // cmp byte [this+0x2448],0 ; reflected FGhostModeRepData (bool at +0, ItemDef at +8, PreviousFocusedSlot +0x10, TimeExited +0x14)

	void* Iface = reinterpret_cast<uint8*>(this) + 0x598; // embedded interface (secondary vtable) inside the controller

	// 1) make sure the harvest tool (EFortItemType::WeaponHarvest = 0x17) is on the quickbars
	if (AFortQuickBars* QB0 = Unresolved_InventoryOwnerInterface_GetQuickBars_vt50(Iface))
	{
		TArray<UObject*> Harvest;
		Unresolved_InventoryOwnerInterface_GetItemsOfType_vtD0(Iface, static_cast<uint8>(EFortItemType::WeaponHarvest), Harvest);
		if (Harvest.Num() > 0) Unresolved_QuickBars_vt630(QB0, Harvest[0], 0, true);
	}

	// 2) resolve the replicated ghost item definition (class test with the class from 0x142b5be40; non-matching => null)
	UObject* Def = GhostModeRepData.GhostModeItemDef;
	if (Def && !Def->GetClass()->IsChildOf(Unresolved_StaticClass_142b5be40())) Def = nullptr;

	// 3) if the item is replicated and already focused in slot 0 of the primary bar we are done; every other outcome re-arms the 0.1s retry
	bool bDone = false;
	if (AFortQuickBars* QB = Unresolved_InventoryOwnerInterface_GetQuickBars_vt50(Iface))
	{
		if (UObject* Item = Unresolved_InventoryOwnerInterface_FindItemByDefinition_vtA0(Iface, Def))
		{
			uint8 FoundBar = 0; int32 FoundSlot = 0;
			if (Unresolved_QuickBars_FindItemSlot_142057010(QB, Item, &FoundBar, &FoundSlot) && FoundSlot != 0)
			{
				QB->SwapItems(EFortQuickBars::Primary, FoundSlot, EFortQuickBars::Primary, 0); // found bar type is ignored by native
			}
			else
			{
				Unresolved_QuickBars_vt628(QB, Item, 0, 0, true);
			}
			Unresolved_QuickBars_vt640(QB, 0, 0, 0.0f, true, true);

			FoundBar = 3; FoundSlot = -1;
			Unresolved_QuickBars_FindItemSlot_142057010(QB, Item, &FoundBar, &FoundSlot); // result byte ignored; out values re-tested below
			if (UObject* Obj14d8 = Unresolved_Controller_vt14d8(this))
			{
				void* Sub = reinterpret_cast<uint8*>(Obj14d8) + 0x4c8;
				if (UObject* Container = Unresolved_SubObject4c8_vt10(Sub))
				{
					uint8 Tmp[16];
					const uint32* Key = Unresolved_Item_vt480(Item, Tmp);
					if (Unresolved_LookupByDword_140708a10(Container, *Key) && FoundBar == 0 && FoundSlot == 0
						&& Unresolved_QuickBars_GetFocusedItem_142059e40(QB) == Item)
					{
						bDone = true; // jumps to the epilogue without scheduling the timer
					}
				}
			}
		}
	}

	if (!bDone)
	{
		// native: TimerManager = (GameInstance ? GameInstance->TimerManager : World->TimerManager); delegate = CreateUObject(this, this-function);
		// SetTimer(local-zero handle (not stored), delegate, 0.1f, false, -1.0f)
		FTimerHandle Handle;
		GetWorldTimerManager().SetTimer(Handle, this, &AFortPlayerControllerGameplay::CheckGhostModeItemReplicated, 0.1f, false);
	}
}
