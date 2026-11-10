// CONFIDENCE: HIGH for the RPC flow, MEDIUM for the implementation (see deps). Validate on one copy of the payload; a failed check raises the RPC validation error, otherwise the implementation runs on another copy.
// Original address: 0x142747b90 (exec). Validate: virtual slot 0xa60 -> 0x141be9bc0. Implementation: virtual slot 0xa68 -> 0x141be9b10.
FFortItemEntry ValidateCopy(OutputItem);
if (!ServerSpawnItem_Validate(ValidateCopy))
{
    RPC_ValidateFailed(TEXT("ServerSpawnItem_Validate"));
}
else
{
    FFortItemEntry ImplementCopy(OutputItem);
    ServerSpawnItem_Implementation(ImplementCopy);
}
