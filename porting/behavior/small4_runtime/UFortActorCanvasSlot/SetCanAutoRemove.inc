// Original address: 0x144d62320
// Evidence: DL stored at +0x3a = bCanAutoRemove (bitfield mask 0x1). Pointer at +0x40 (NativeRecoveredUint64_0x40) when non-null receives DL at pointee+0x38 (raw).
// CONFIDENCE: MEDIUM
bCanAutoRemove = bAllowAutoRemove;
if (NativeRecoveredUint64_0x40 != 0)
{
	uint8* Target = reinterpret_cast<uint8*>(NativeRecoveredUint64_0x40);
	Target[0x38] = static_cast<uint8>(bAllowAutoRemove);
}
