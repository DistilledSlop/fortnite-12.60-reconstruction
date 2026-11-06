// Behaviour snippet: AFortVolumeManager::GetVolumeForLocation (body only; spliced into the generated member function).
// Original address: 0x142b3be00
// Decompile source: porting/generated/native235-fresh-bulk/0x142b3be00.c
// Inferred: Thin wrapper: the FVector parameter is stepped into a local and passed by reference to the helper at 0x141c7cde0 (NativeFindVolumeAtLocation, ported); its result is returned.
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
return NativeFindVolumeAtLocation(Location);
