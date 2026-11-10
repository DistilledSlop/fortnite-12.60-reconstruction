// CONFIDENCE: MEDIUM
// Original address: 0x142a1ee20 (exec thunk). Dispatches through virtual slot +0xbd0, which is
// AFortPawn::PickUpActor_Implementation (native body 0x14233f230, dep AFortPawn__PickUpActor_Implementation, PARTIAL).
// Script read order: PickupTarget (object), then PlacementDecoItemDefinition (object). The implementation uses the
// second value as the item definition passed to EquipWeaponDefinition, so the reflected name looks misleading; confirm.
ReturnValue = PickUpActor_Implementation(PickupTarget, PlacementDecoItemDefinition);
