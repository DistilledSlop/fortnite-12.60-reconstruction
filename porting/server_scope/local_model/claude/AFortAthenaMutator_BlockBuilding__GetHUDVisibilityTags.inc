// CONFIDENCE: HIGH. Original address: 0x1427f5df0 (exec thunk). Virtual target: vslot 0xcb8 -> 0x141aad100 (GetHUDVisibilityTags body).
// Behaviour: the BlockBuilding HUD exposes no gameplay tags; the native body clears the out container and returns it.
// The exec only copies the virtual's result into the return buffer (FGameplayTagContainer_CopyAssign).
return GetHUDVisibilityTags_Implementation();
