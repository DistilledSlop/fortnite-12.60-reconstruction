// Behaviour snippet: AFortPlayerPawn::OnRep_AccelerationZPack (body only; spliced into the generated member function).
// Thunk address: 0x142a7dbb0 (exec wrapper, no declared params).
// Original body address: 0x1420ee770 (real 57-byte writer, shared with OnRep_AccelerationPack; not a stub).
// Evidence: thunk passes RCX = *(this+0x280) and RDX = local 12-byte struct {qword from Sub+0x1e2c, float Z}, where
// Z = clamp((int8)this[0x1068] + 127) / 254, then Z * 8192 - 4096. Body writes Sub+0x22c/+0x234/+0x1e2c/+0x1e34,
// sets bit 0x8 at Sub+0x1e38 and bit 0x40 at Sub+0x38b. Sub is the sub-object at +0x280 (class unresolved), so raw offsets.
// CONFIDENCE: MEDIUM
uint8* Raw = reinterpret_cast<uint8*>(this);
uint8* Sub = *reinterpret_cast<uint8**>(Raw + 0x280);
uint8 Pack[12];
FMemory::Memcpy(Pack, Sub + 0x1e2c, 8);
float Norm = ((float)(int)*reinterpret_cast<int8*>(Raw + 0x1068) + 127.0f) * 0.003937008f;
float Clamped = 0.0f;
if (Norm >= 0.0f)
{
	Clamped = Norm;
	if (Norm >= 1.0f)
	{
		Clamped = 1.0f;
	}
}
float Z = Clamped * 8192.0f - 4096.0f;
FMemory::Memcpy(Pack + 8, &Z, 4);
FMemory::Memcpy(Sub + 0x22c, Pack, 8);
FMemory::Memcpy(Sub + 0x234, Pack + 8, 4);
FMemory::Memcpy(Sub + 0x1e2c, Pack, 8);
FMemory::Memcpy(Sub + 0x1e34, Pack + 8, 4);
Sub[0x1e38] |= 0x8;
Sub[0x38b] |= 0x40;
