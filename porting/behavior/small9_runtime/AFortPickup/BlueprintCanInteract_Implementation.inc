// Behaviour snippet: AFortPickup::BlueprintCanInteract_Implementation (body only; spliced into the generated function).
// Original body address: 0x1420279b0. Offset evidence: no object member access. MOV byte [R8],0 writes bCanInteractOverride (third ABI argument, bool&); XOR AL,AL returns false.
// CONFIDENCE: HIGH
bCanInteractOverride = false;
return false;
