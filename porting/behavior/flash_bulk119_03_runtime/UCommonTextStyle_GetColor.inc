// CONFIDENCE: high
// Native helper 0x1407c1f80 reads the upper half first, then copies the lower half.
uint64 Upper;
uint64 Lower;
FMemory::Memcpy(&Upper, reinterpret_cast<const uint8*>(&Color) + 8, 8);
FMemory::Memcpy(&Lower, &Color, 8);
FMemory::Memcpy(&OutColor, &Lower, 8);
FMemory::Memcpy(reinterpret_cast<uint8*>(&OutColor) + 8, &Upper, 8);
