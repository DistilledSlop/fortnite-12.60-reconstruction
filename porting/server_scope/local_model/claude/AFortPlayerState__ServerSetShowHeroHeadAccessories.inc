// CONFIDENCE: HIGH. Exec thunk: reads bShow, runs the RPC _Validate (vslot 0x718 -> 0x1406801d0, STUB Stub_ReturnTrue, so the validate-failed branch is unreachable), then _Implementation (vslot 0x720 -> 0x142427790).
// Original address: 0x142a89ba0 (exec). Validate: 0x1406801d0 (STUB). Implementation: 0x142427790 (see deps AFortPlayerState__ServerSetShowHeroHeadAccessories__Implementation.json).
if (!ServerSetShowHeroHeadAccessories_Validate(bShow))
{
    RPC_ValidateFailed(TEXT("ServerSetShowHeroHeadAccessories_Validate"));
}
else
{
    ServerSetShowHeroHeadAccessories_Implementation(bShow);
}
