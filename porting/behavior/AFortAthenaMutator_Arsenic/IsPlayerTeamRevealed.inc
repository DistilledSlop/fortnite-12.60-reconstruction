// Original address: 0x1427e15d0 (exec thunk; body 0x141ab16d0).
// Decompile source: porting/generated/native107-impl/0x141ab16d0.c
// CONFIDENCE: HIGH
// Body: scans the array at this+0x668 (stride 0x20, count NativeRecoveredInt_0x670) for the entry whose pointer at +0x10
// equals PlayerState and returns the flag byte at +0x18 as a bool. Returns false when no entry matches.
const uint8* Begin = reinterpret_cast<const uint8*>(NativeRecoveredPointer_0x668);
const uint8* End = Begin + static_cast<int64>(NativeRecoveredInt_0x670) * 0x20;
for (const uint8* Entry = Begin; Entry != End; Entry += 0x20)
{
    if (reinterpret_cast<const void*>(*reinterpret_cast<const uint64*>(Entry + 0x10)) == static_cast<const void*>(PlayerState))
    {
        return *(Entry + 0x18) != 0;
    }
}
return false;
