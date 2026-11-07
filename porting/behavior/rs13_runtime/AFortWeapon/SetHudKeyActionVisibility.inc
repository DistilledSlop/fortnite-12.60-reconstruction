// Original address: 0x1426b3020 (thunk 0x142b485a0). Offset evidence: HudKeyActionData TArray at +0xb60 (Num at +0xb68, reflected); element bVisible is at element +0x28 with 0x30 stride (asm, element layout of FWeaponHudData not offset-verified: header reports bVisible offset None); OnWeaponHudKeyActionUpdated delegate at +0xb50 (reflected) broadcast with this as its single packed param.
// CONFIDENCE: MEDIUM
if (Index < 0 || Index >= HudKeyActionData.Num())
{
    return;
}
FWeaponHudData& Action = HudKeyActionData[Index];
if (Action.bVisible == bVisible)
{
    return;
}
Action.bVisible = bVisible;
OnWeaponHudKeyActionUpdated.Broadcast(this);
