// CONFIDENCE: HIGH. Complete original body; build/runtime pending.
// Original base backend +0x30: direct Groups+0x58, never virtual +0x28.
OutItems.Empty(0);
if (const FMcpProfileItemArray* Group = NativeFindProfileGroup(Groups, Key))
{
    OutItems = *Group;
}
