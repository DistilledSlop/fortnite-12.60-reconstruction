// Behaviour snippet: UAthenaMarkerComponent::FindMarkerByID (body only).
// Original body address: 0x141a770f0 (thunk 0x142725850), receiver delta 0.
// Evidence: marker records are 0x100 bytes starting at NativeRecoveredPtr_0x240 with count NativeRecoveredInt32_0x248 (both unreflected, own region). Each record holds a marker object pointer at +0xf8. MarkerID is 8 bytes: low dword compared with marker+0x3c, high dword with marker+0x40 (raw offsets; the object's reflected MarkerDataCache at +0x30 has no known inner layout). Returns the first marker with a null-checked match, else null.
// CONFIDENCE: MEDIUM
const int32* IdWords = reinterpret_cast<const int32*>(&MarkerID);
uint8* Entry = reinterpret_cast<uint8*>(NativeRecoveredPtr_0x240);
uint8* End = Entry + static_cast<int64>(NativeRecoveredInt32_0x248) * 0x100;
for (; Entry != End; Entry += 0x100)
{
    uint8* Marker = *reinterpret_cast<uint8* const*>(Entry + 0xf8);
    if (Marker != nullptr && *reinterpret_cast<const int32*>(Marker + 0x3c) == IdWords[0] && *reinterpret_cast<const int32*>(Marker + 0x40) == IdWords[1])
    {
        return reinterpret_cast<UFortWorldMarker*>(Marker);
    }
}
return nullptr;
