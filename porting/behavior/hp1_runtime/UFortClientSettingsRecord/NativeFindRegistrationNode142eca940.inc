// Helper 0x142eca940, host UFortClientSettingsRecord (static).
// Evidence: walks the singly linked list at Context+0x20 (next pointer at node+0x20). For each node, holder = node+0x10;
// holder Data at +0x30 and Num at +0x38. A node matches when Data[KeyRecord int32 +0x38] == KeyRecord+0x30; first match wins.
// Callers: UFortAudioEventForwarder::HandleGamePhaseChanged, UFortClientSettingsRecord GetCrossplayPreference and subtitle setters.
// CONFIDENCE: MEDIUM
const int32 KeyIndex = *reinterpret_cast<const int32*>(KeyRecord + 0x38);
const uint8* KeySlotRaw = KeyRecord + 0x30;
uint8* NodeRaw = *reinterpret_cast<uint8* const*>(reinterpret_cast<const uint8*>(Context) + 0x20);
while (NodeRaw)
{
    uint8* HolderRaw = *reinterpret_cast<uint8* const*>(NodeRaw + 0x10);
    if (KeyIndex <= *reinterpret_cast<const int32*>(HolderRaw + 0x38))
    {
        uint8* SlotArrayRaw = *reinterpret_cast<uint8* const*>(HolderRaw + 0x30);
        if (*reinterpret_cast<const uint8* const*>(SlotArrayRaw + static_cast<int64>(KeyIndex) * 8) == KeySlotRaw)
        {
            return NodeRaw;
        }
    }
    NodeRaw = *reinterpret_cast<uint8* const*>(NodeRaw + 0x20);
}
return nullptr;
