// Behaviour snippet: AFortVolumeManager::GetVolumeForPlayerState (body only; spliced into the generated member function).
// Original address: 0x142b3c000
// Decompile source: porting/generated/native235-fresh-bulk/0x142b3c000.c
// Inferred: Thin wrapper: the PlayerState parameter is stepped, then the helper at 0x141c7d420 is called and its result returned (NativeFindPlayerVolume, ported).
// Declared in generated header: no (the generator emits this declaration when the behaviour splice is on)
// CONFIDENCE: HIGH
return NativeFindPlayerVolume(PlayerState);
