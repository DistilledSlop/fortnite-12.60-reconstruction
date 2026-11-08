// Helper 0x140682990 (UFortHomebaseNodeItemUtilities). Windows x64: RCX = Count (int32), RDX = Chars (uint16*).
// Asm evidence: loop over Count UTF-16 units. Each unit folds 'a'..'z' to 'A'..'Z' (LEA -0x61 / CMP 0x1a / SBB / AND 0x20 / SUB),
// then is consumed as two bytes (low, then high) by the update crc = T[(crc ^ b) & 0xff] ^ (crc >> 8), seed 0, no final xor.
// Callers: AFortFXManager, UFortMcpContext, AFortMission, UFortMissionLibrary, UFortniteAutomationBlueprintLibrary pass (len, wide string).
// Table 0x147c34cf0 is in writable .data (no data_consts were supplied). It was checked against the PE image: all 256 entries equal the
// MSB-first CRC-32 table for poly 0x04C11DB7 (entry i = byte i << 24 shifted 8 times), so it is regenerated here.
// CONFIDENCE: MEDIUM
static const uint32* const CrcTable = []() -> const uint32*
{
    static uint32 CrcTableStorage[256];
    for (int32 TableIndex = 0; TableIndex < 256; ++TableIndex)
    {
        uint32 TableEntry = static_cast<uint32>(TableIndex) << 24;
        for (int32 TableBit = 0; TableBit < 8; ++TableBit)
        {
            TableEntry = (TableEntry & 0x80000000u) ? ((TableEntry << 1) ^ 0x04C11DB7u) : (TableEntry << 1);
        }
        CrcTableStorage[TableIndex] = TableEntry;
    }
    return CrcTableStorage;
}();

uint32 CrcValue = 0;
for (int32 CharIndex = 0; CharIndex < Count; ++CharIndex)
{
    uint16 FoldedChar = Chars[CharIndex];
    if (FoldedChar >= 'a' && FoldedChar <= 'z')
    {
        FoldedChar = static_cast<uint16>(FoldedChar - 0x20);
    }
    const uint8 LowByte = static_cast<uint8>(FoldedChar & 0xff);
    const uint8 HighByte = static_cast<uint8>(FoldedChar >> 8);
    CrcValue = CrcTable[(CrcValue ^ LowByte) & 0xff] ^ (CrcValue >> 8);
    CrcValue = CrcTable[(CrcValue ^ HighByte) & 0xff] ^ (CrcValue >> 8);
}
return CrcValue;
