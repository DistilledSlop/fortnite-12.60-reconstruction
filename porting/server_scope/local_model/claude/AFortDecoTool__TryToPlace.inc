// CONFIDENCE: LOW
// Original addresses: exec thunk 0x1428d7c20 (BP-visible bool result, forwards FUN_1426b8990's rax); body 0x1426b8990.
// The body decompiles as void, so the bool the thunk returns is not recoverable from the package. The port keeps the
// placement flow; the return value is set from the CanTryToPlace gate that guards the whole body.
bool bTryPlaced = CanTryToPlace();                               // virtual slot 0xb08 (target not resolved in this pass)
if (bTryPlaced)
{
	AFortPlayerController* PC = GetInstigatorController();      // 0x14269e2e0
	UObject* Helper = DecoHelper;                                // this->DecoHelper (+0xb98)
	if (PC != nullptr && Helper != nullptr)
	{
		UFortDecoItemDefinition* DecoItemDef = GetDecoItemDefinition();    // virtual 0xb00 -> 0x14269bb00
		if (!HelperAcceptsPlacement(Helper, false) || DecoItemDef == nullptr)   // 0x142678960 (not captured)
		{
			CancelPlacement(PC, Helper);                         // 0x142388df0 (not captured)
		}
		else
		{
			// Copy the helper's placement transforms (0x14266c030 and 0x14266c020; FUN_142c5d000 copies the second one).
			FTransform HelperTransform = GetHelperTransform(Helper);            // 0x14266c030
			FTransform HelperTransformCopy = GetHelperTransform(Helper);
			bool bItemFlag = DecoItemDefFlagA(DecoItemDef);                     // 0x14201dfa0
			UObject* Extra = bItemFlag ? GetHelperExtra(Helper) : nullptr;      // 0x14266c020

			if (!PC->bFlag_7aa || Extra != nullptr || (DecoItemDef->bDecoFlags_b35 & 2) == 0)
			{
				// direct placement path: virtual 0xad8 (target not resolved in this pass)
				DirectPlaceDeco(HelperTransform, HelperTransformCopy, (uint8)Helper->Field234);   // 0xad8
			}
			else
			{
				// alternate path: virtual 0xae0 (target not resolved in this pass), uses the helper's sub-object at 0x141363fb0
				AlternatePlaceDeco(GetHelperSubobject(Helper), HelperTransform, Helper->Field234);   // 0xae0
			}
			SetPlacementActive(false);                           // virtual 0x688 with arg 0 (target not resolved)

			// Gadget bookkeeping: when ItemDefinition is a FortGadgetItemDefinition, notify the controller inventory.
			UFortDecoItemDefinition* ItemDef = ItemDefinition;   // this->ItemDefinition (+0xb90)
			if (ItemDef != nullptr && ItemDef->IsA(UFortGadgetItemDefinition::StaticClass()))   // 0x1428e9db0
			{
				GadgetNotifyPlacement(ItemDef, PC);             // 0x142cea230 / 0x142659400 / 0x1417a0fd0 / 0x142027b30 (not captured)
			}
		}
	}
}
