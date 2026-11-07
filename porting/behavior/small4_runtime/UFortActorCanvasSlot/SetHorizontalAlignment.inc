// Original address: 0x144d63d60
// Evidence: DL stored as byte at +0x38 = HorizontalAlignment. Pointer at +0x40 (NativeRecoveredUint64_0x40) when non-null receives EDX as int32 at pointee+0 (raw).
// CONFIDENCE: MEDIUM
HorizontalAlignment = InHorizontalAlignment;
if (NativeRecoveredUint64_0x40 != 0)
{
	int32* Target = reinterpret_cast<int32*>(NativeRecoveredUint64_0x40);
	*Target = static_cast<int32>(InHorizontalAlignment);
}
