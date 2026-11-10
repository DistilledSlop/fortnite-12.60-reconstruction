// CONFIDENCE: HIGH for dispatch, MEDIUM for parameter order (reverse-of-reflection read order).
// Original exec thunk 0x142a4cb50. Server RPC receiver: validate, then implementation.
// Validate slot 0x1278 resolves to a real validate (0x1423ac030), read in full.
if (!ServerRequestAttributeSources_Validate(Attribute, AbilitySystemComponent))
{
    RPC_ValidateFailed(TEXT("ServerRequestAttributeSources_Validate"));
    return;
}
ServerRequestAttributeSources_Implementation(Attribute, AbilitySystemComponent);   // slot 0x1280 -> 0x1423abf70
