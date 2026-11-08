// Helper 0x141b48bf0, host AFortAthenaMutator_Wax (static). RDX is the output record buffer; RAX returns it.
// Evidence: scans 0x48-byte records from [Context+0x1368] (int32 Num at +0x1370) for qword +0x10 == RecordKey; RecordKey 0 never matches.
// Found: out qword +0x10 = key; out bytes +0x18 and +0x19 copied; out dwords +0x1c..+0x38 copied; out qword +0x3c and dword +0x44 copied;
//        out bytes 0..0xb = 0xFF. Bytes 0xc..0xf, 0x1a..0x1b are left untouched.
// Not found: out bytes 0..0xb = 0xFF and bytes 0xc..0x47 = 0.
// CONFIDENCE: HIGH
const uint8* ContextRaw = reinterpret_cast<const uint8*>(Context);
const uint8* RecordCursor = *reinterpret_cast<const uint8* const*>(ContextRaw + 0x1368);
const int32 RecordCount = *reinterpret_cast<const int32*>(ContextRaw + 0x1370);
const uint8* RecordEnd = RecordCursor + static_cast<int64>(RecordCount) * 0x48;
const uint8* MatchRaw = nullptr;
if (RecordKey != 0)
{
    while (RecordCursor != RecordEnd)
    {
        if (*reinterpret_cast<const uint64*>(RecordCursor + 0x10) == RecordKey)
        {
            MatchRaw = RecordCursor;
            break;
        }
        RecordCursor += 0x48;
    }
}
if (!MatchRaw)
{
    FMemory::Memset(OutRecord, 0xFF, 0xC);
    FMemory::Memzero(OutRecord + 0xC, 0x3C);
    return OutRecord;
}
*reinterpret_cast<uint64*>(OutRecord + 0x10) = RecordKey;
OutRecord[0x18] = MatchRaw[0x18];
OutRecord[0x19] = MatchRaw[0x19];
FMemory::Memcpy(OutRecord + 0x1C, MatchRaw + 0x1C, 0x20);
FMemory::Memcpy(OutRecord + 0x3C, MatchRaw + 0x3C, 8);
FMemory::Memcpy(OutRecord + 0x44, MatchRaw + 0x44, 4);
FMemory::Memset(OutRecord, 0xFF, 0xC);
return OutRecord;
