// Original body 0x142703390 (thunk 0x142ace4c0). Offset evidence: byte flag at +0x5a9 (member NativeRecoveredBool_0x5a9, unreflected gap); TArray DisabledGameplayMessages at +0x630 (Num at +0x638, reflected); element is 16 bytes {MessageOwnerClassName at +0, MessageName at +8}, matching FRuntimeOptionDisabledGameplayMessage.
// Returns false when the flag is set or an element matches both names; returns true otherwise.
// CONFIDENCE: HIGH
if (NativeRecoveredBool_0x5a9)
{
    return false;
}
for (const FRuntimeOptionDisabledGameplayMessage& Entry : DisabledGameplayMessages)
{
    if (Entry.MessageOwnerClassName == MessageOwnerClassName && Entry.MessageName == MessageName)
    {
        return false;
    }
}
return true;
