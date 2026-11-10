// CONFIDENCE: HIGH. Exec thunk: reads bShow, runs the RPC _Validate (vslot 0x708 -> 0x1406801d0, STUB Stub_ReturnTrue, so the validate-failed branch is unreachable), then _Implementation (vslot 0x710 -> 0x142427750).
// Original address: 0x142a89ae0 (exec). Validate: 0x1406801d0 (STUB). Implementation: 0x142427750 (see deps AFortPlayerState__ServerSetShowHeroBackpack__Implementation.json).
if (!ServerSetShowHeroBackpack_Validate(bShow))
{
    RPC_ValidateFailed(TEXT("ServerSetShowHeroBackpack_Validate"));
}
else
{
    ServerSetShowHeroBackpack_Implementation(bShow);
}
