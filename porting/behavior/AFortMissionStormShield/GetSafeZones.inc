// Behaviour snippet: AFortMissionStormShield::GetSafeZones (body only; spliced into the generated member function).
// Original address: 0x1429efd10 (reflected exec, out TArray parameter)
// Decompile source: porting/generated/native108-broad/0x1429efd10.c
// Inferred: copies SafeAreas (TArray data pointer at +0x430, count at +0x438) into the out parameter; a self-assignment is skipped by the decompile.
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
OutSafeZones = SafeAreas;
