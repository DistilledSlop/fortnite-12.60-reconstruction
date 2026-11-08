// Helper 0x140769940: TSet<T>::operator=(const TSet<T>&) for an 8-byte, bitwise-copyable element type T; returns Dest in RAX.
// Evidence: CMP RCX,RDX self-assign guard; sparse-array data Num/Max at +8/+0xc, element stride 0x10 (8-byte value + HashNextId + HashIndex)
//   copied with memcpy; TBitArray with 4-dword inline storage at +0x10 (NumBits +0x28, MaxBits +0x2c, secondary data ptr +0x20);
//   FirstFreeIndex/NumFreeIndices at +0x30/+0x34; hash TInlineAllocator<1> (inline +0x38, ptr +0x40) with HashSize +0x48 copied by memcpy
//   of HashSize*4 bytes. All growth helpers (0x140690920/0x1406911b0/0x140690310/0x140690dd0) are ResizeAllocation/Reserve of those
//   containers. Elements are copied as raw bytes, so one 8-byte trivially-copyable element type reproduces the layout and behaviour.
// CONFIDENCE: MEDIUM
TSet<uint64>* DestSet = static_cast<TSet<uint64>*>(Dest);
const TSet<uint64>* SourceSet = static_cast<const TSet<uint64>*>(Source);
*DestSet = *SourceSet;
return Dest;
