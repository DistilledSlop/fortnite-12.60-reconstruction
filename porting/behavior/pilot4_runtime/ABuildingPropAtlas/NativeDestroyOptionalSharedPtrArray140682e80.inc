// Helper 0x140682e80 (47 insns): RCX = &array {Data at +0, Num at +8, Max at +0xc}.
// Per element (stride 0x30): flag byte at +0x28; when set it is cleared and the shared reference whose controller is at
// +0x18 (object pointer at +0x10) is released with the LOCK XADD / vtable[0] / vtable[1] sequence = TSharedPtr<T, ThreadSafe>::Reset.
// After the loop the buffer [RCX] is freed (tail jump to FMemory::Free 0x142bee710), Data/Num are left untouched.
// CONFIDENCE: MEDIUM
uint8* ArrayData = *reinterpret_cast<uint8**>(ArrayStorage);
const int32 ArrayNum = *reinterpret_cast<const int32*>(ArrayStorage + 0x8);
for (int32 ElementIndex = 0; ElementIndex < ArrayNum; ++ElementIndex)
{
    uint8* Element = ArrayData + static_cast<SIZE_T>(ElementIndex) * 0x30;
    if (Element[0x28] != 0)
    {
        Element[0x28] = 0;
        reinterpret_cast<TSharedPtr<uint8, ESPMode::ThreadSafe>*>(Element + 0x10)->Reset();
    }
}
if (ArrayData)
{
    FMemory::Free(ArrayData);
}
