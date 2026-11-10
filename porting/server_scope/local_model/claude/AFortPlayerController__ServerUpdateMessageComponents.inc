// CONFIDENCE: HIGH - RPC exec thunk: _Validate gate, then _Implementation.
// Original exec 0x142a4e7b0. Validate: vtable +0x13f0 = 0x1423ae240. Implementation: vtable +0x13f8 = 0x1423ae010.
// FFrame read order is ModifiedComponents, then NewChannelIds (inventory parameter list is reversed).
if (!ServerUpdateMessageComponents_Validate(ModifiedComponents, NewChannelIds))
{
    RPC_ValidateFailed(TEXT("ServerUpdateMessageComponents_Validate"));
}
else
{
    ServerUpdateMessageComponents_Implementation(ModifiedComponents, NewChannelIds);
}
