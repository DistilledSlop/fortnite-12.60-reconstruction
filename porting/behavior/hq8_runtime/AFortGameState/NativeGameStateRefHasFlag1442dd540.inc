// Helper 0x1442dd540 (AFortGameState): MOV RAX,[RCX+0x128]; if RAX && (byte [RAX+0x1f4] & 0x20) return true.
// Offset 0x128 is read as an 8-byte pointer (qword compare in callers); the reflected AActor::Children
// Data/Num/Max overlap there has a different width, so raw access is used.
// The referenced object's class is not established by the callers, so raw byte access is used at +0x1f4.
// CONFIDENCE: MEDIUM
UObject* const ReferencedObject = *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(GameState) + 0x128);
if (ReferencedObject != nullptr && (*(reinterpret_cast<const uint8*>(ReferencedObject) + 0x1f4) & 0x20) != 0)
{
    return true;
}
return false;
