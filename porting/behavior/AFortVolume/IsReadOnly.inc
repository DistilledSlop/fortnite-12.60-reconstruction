// Behaviour snippet: AFortVolume::IsReadOnly (body only; spliced into the generated member function).
// Original address: 0x142b37a30
// Decompile source: porting/generated/native-triage011/0x142b37a30.c
// Inferred: Reads VolumeState at +0x408 (enum EVolumeState, value 1 is ReadOnly) and bit 0x1 of the byte at +0x2d8, which bool_bits.py maps to bNeverAllowSaving.
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
return VolumeState == EVolumeState::ReadOnly || bNeverAllowSaving;
