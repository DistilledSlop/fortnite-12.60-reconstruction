// Original address: 0x14282e0f0 (exec thunk; body 0x141b492b0).
// Decompile source: porting/generated/native107-impl/0x141b492b0.c (thunk: native107-broad/0x14282e0f0.c)
// CONFIDENCE: HIGH
// Body: a null PlayerState returns 0. Otherwise it walks the outer array at this+0x610 (16-byte pairs: element
// pointer, element count) for NativeRecoveredInt_0x618 pairs and counts the pairs whose element list contains PlayerState.
if (PlayerState == nullptr)
{
    return 0;
}
int32 Score = 0;
const uint64* Groups = reinterpret_cast<const uint64*>(NativeRecoveredPointer_0x610);
const int32 GroupCount = NativeRecoveredInt_0x618;
for (int32 GroupIndex = 0; GroupIndex < GroupCount; ++GroupIndex)
{
    const uint64* Elements = reinterpret_cast<const uint64*>(Groups[GroupIndex * 2]);
    const int32 ElementCount = static_cast<int32>(Groups[GroupIndex * 2 + 1]);
    bool bContains = false;
    for (int32 ElementIndex = 0; ElementIndex < ElementCount; ++ElementIndex)
    {
        if (reinterpret_cast<const void*>(Elements[ElementIndex]) == static_cast<const void*>(PlayerState))
        {
            bContains = true;
            break;
        }
    }
    if (bContains)
    {
        ++Score;
    }
}
return Score;
