// Original body address: 0x141b5af50 (helper, no thunk; returns Self in RAX)
// Offset evidence: all writes are relative to RCX (Self). Float stores go to +0xa8, +0xac, +0xb0, +0xb4, +0xb8, +0xbc, +0xc0 from float arguments 2..8 (XMM1..3, then stack slots 5..8). Seven 0x18-byte groups at +0x0, +0x18, ..., +0x90 get zero at +0 and +8 and 2 at +0x10. Qwords are zeroed at +0xc4 and +0xcc and a dword at +0xd4.
// Caller GetCurrentBoostTimer (0x1426fe870) passes a static global (0x14816fe10) as Self, inside a one-time static initialization guard. The helper therefore initializes its first argument and returns it.
// The 0x18-byte groups and the float fields have no reflected names, so they are written at raw offsets.
// CONFIDENCE: MEDIUM
for (int32 GroupIndex = 0; GroupIndex < 7; ++GroupIndex)
{
	uint8* GroupBase = Self + GroupIndex * 0x18;
	*reinterpret_cast<uint64*>(GroupBase + 0x00) = 0;
	*reinterpret_cast<uint64*>(GroupBase + 0x08) = 0;
	*reinterpret_cast<uint64*>(GroupBase + 0x10) = 2;
}
*reinterpret_cast<uint64*>(Self + 0xc4) = 0;
*reinterpret_cast<uint64*>(Self + 0xcc) = 0;
*reinterpret_cast<int32*>(Self + 0xd4) = 0;
*reinterpret_cast<float*>(Self + 0xa8) = FloatArg2;
*reinterpret_cast<float*>(Self + 0xac) = FloatArg3;
*reinterpret_cast<float*>(Self + 0xb0) = FloatArg4;
*reinterpret_cast<float*>(Self + 0xb4) = FloatArg5;
*reinterpret_cast<float*>(Self + 0xb8) = FloatArg6;
*reinterpret_cast<float*>(Self + 0xbc) = FloatArg7;
*reinterpret_cast<float*>(Self + 0xc0) = FloatArg8;
return Self;
