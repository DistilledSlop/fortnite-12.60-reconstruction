// Helper 0x14468f240: count-gated weak-pointer resolve.
// Asm: CMP dword [RCX+0x1d0],0 ; JLE -> 0; else RCX=[RCX+0x1c8] and tail-JMP 0x142ee5dd0 (TWeakObjectPtr::Get, per callee hint).
// No reflected member covers 0x1c8 or 0x1d0 (lookup_offset.py AUnicornDriver), so raw access is used.
// Confidence MEDIUM: the asm passes the loaded 8-byte value in RCX to the Get hint; the value is wrapped as TWeakObjectPtr<UObject> before Get().
// CONFIDENCE: MEDIUM
const uint8* DriverBytes = reinterpret_cast<const uint8*>(Driver);
const int32 ActiveCount = *reinterpret_cast<const int32*>(DriverBytes + 0x1d0);
if (ActiveCount <= 0)
{
    return nullptr;
}
const TWeakObjectPtr<UObject> WeakTarget = *reinterpret_cast<const TWeakObjectPtr<UObject>*>(DriverBytes + 0x1c8);
return WeakTarget.Get();
