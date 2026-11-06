// Original address: 0x1427e1350 (exec thunk; body 0x141aaeca0).
// Decompile source: porting/generated/native107-impl/0x141aaeca0.c
// CONFIDENCE: HIGH
// Body: scans the array at this+0x550 (stride 0x18, count NativeRecoveredInt_0x558) for the entry whose byte at +0xc equals
// Team and returns the int32 at +0x10. Returns -1 when no entry matches.
const uint8* Begin = reinterpret_cast<const uint8*>(NativeRecoveredPointer_0x550);
const uint8* End = Begin + static_cast<int64>(NativeRecoveredInt_0x558) * 0x18;
for (const uint8* Entry = Begin; Entry != End; Entry += 0x18)
{
    if (Entry[0xc] == Team)
    {
        return *reinterpret_cast<const int32*>(Entry + 0x10);
    }
}
return -1;
