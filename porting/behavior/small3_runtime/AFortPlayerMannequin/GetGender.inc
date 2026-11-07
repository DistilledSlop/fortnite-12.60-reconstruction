// Original body: 0x141dcbb10 (thunk 0x142a7c170)
// Offset evidence: MOV RAX,[RCX+0x308] (CharacterPartList[0]); null -> MOV EAX,3; else MOVZX EAX,byte [RAX+0x30]
// CONFIDENCE: MEDIUM
const uint8* PartRaw = reinterpret_cast<const uint8*>(CharacterPartList[0]);
if (PartRaw == nullptr)
{
    return static_cast<EFortCustomGender::Type>(3);
}
return static_cast<EFortCustomGender::Type>(PartRaw[0x30]);
