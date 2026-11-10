// CONFIDENCE: MEDIUM
// Original: exec thunk 0x142a83990. Reflection params: bClearInteraction (Bool, 0), InteractingItem (Object, 1).
// The thunk reads InteractingItem first, then bClearInteraction, and calls (this, InteractingItem, bClearInteraction).
// _Validate is virtual slot 0x1418 which resolves to Stub_ReturnTrue 0x1406801d0 (STUB): it always returns true.
// _Implementation is virtual slot 0x1420 which resolves to 0x141a231e0 (dep: deps/AFortPlayerPawnAthena__ServerSetInteractingItem__ServerSetInteractingItem_Implementation.json).
if (!ServerSetInteractingItem_Validate(InteractingItem, bClearInteraction))
{
    RPC_ValidateFailed(TEXT("ServerSetInteractingItem_Validate"));
    return;
}
ServerSetInteractingItem_Implementation(InteractingItem, bClearInteraction);
