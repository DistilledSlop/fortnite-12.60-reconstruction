// CONFIDENCE: HIGH. Complete original body; build/runtime pending.
// Original overlay +0x30: reset before calling the backend's +0x28 virtual.
OutItems.Reset(0);
if (const FMcpProfileItemArray* Group = Backend.FindItemsByTemplateKey(Key))
{
    OutItems = *Group;
}
