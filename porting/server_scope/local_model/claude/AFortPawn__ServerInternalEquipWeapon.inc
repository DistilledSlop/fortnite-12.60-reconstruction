// CONFIDENCE: HIGH
// Original address: 0x142a1f670 (exec thunk). Server RPC: validate first through virtual slot +0x8e8
// (AFortPawn::ServerInternalEquipWeapon_Validate, dep), then dispatch the implementation through slot +0x8f0
// (AFortPawn::ServerInternalEquipWeapon_Implementation, dep).
// Script parameter: Weap (object).
if (!ServerInternalEquipWeapon_Validate(Weap))
{
    RPC_ValidateFailed(L"ServerInternalEquipWeapon_Validate");
    return;
}
ServerInternalEquipWeapon_Implementation(Weap);
