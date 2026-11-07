// Original address: 0x144d66aa0
// Evidence: DL stored as byte at +0x39 = VerticalAlignment. Pointer at +0x40 (NativeRecoveredUint64_0x40) when non-null receives EDX as int32 at pointee+0x4 (raw).
// CONFIDENCE: MEDIUM
VerticalAlignment = InVerticalAlignment;
if (NativeRecoveredUint64_0x40 != 0)
{
	int32* Target = reinterpret_cast<int32*>(reinterpret_cast<uint8*>(NativeRecoveredUint64_0x40) + 0x4);
	*Target = static_cast<int32>(InVerticalAlignment);
}
