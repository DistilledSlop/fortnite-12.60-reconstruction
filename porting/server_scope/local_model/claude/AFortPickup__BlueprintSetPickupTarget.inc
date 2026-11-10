// CONFIDENCE: MEDIUM
// Original: exec thunk 0x142a2a600; native body 0x1420279f0. FFrame read order: PickupTarget, bPlayPickupSound.
// Null target is a no-op.
if (!PickupTarget)
{
    return;
}

// Random scatter direction (0x14203e6d0, ported in deps) and speed (0x14202dc10, deps only partly read).
const FVector ScatterDir = RandomUnitVector();
const float ScatterSpeed = GetRandomScatterSpeed();

// Virtual slot 0x640 -> 0x142038d10 (334-line body, NOT PORTED; see deps AFortPickup__SetPickupTargetNative).
SetPickupTargetNative(PickupTarget, ScatterSpeed, ScatterDir, bPlayPickupSound);
