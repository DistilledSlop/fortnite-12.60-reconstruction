// CONFIDENCE: MEDIUM. Original addresses: exec 0x142a7da20 dispatches via vtable slot 0xe48 to native body 0x1423376f0 (54 lines, read in full). Helpers read to depth two; LOW items are the virtual slots 0xb90, 0xcf0, 0x810 and 0x2268 (targets not captured).
NativeRecoveredByte_0x10e6 = 0;                        // pawn+0x10e6 := 0 (parachute movement no longer active)
UpdateParachuteStateFromMovement();                    // 0x14235bbf0 (read in full)
ParachuteVirtual_0x1208();                             // vtable 0x1208 on this (LOW name)
CacheParachuteFallSpeedFromPawn(CharacterMovement);    // 0x1420edbc0 (read in full)
if (NativeRecoveredPtr_0x2490 == nullptr)
{
    if (!(NativeRecoveredByte_0x320 & 8))
    {
        SetParachuteCosmeticsAsync(false);             // 0x142332fe0 with false (read in full)
    }
}
else
{
    DisableParachuteCosmetics();                       // 0x141a1c3c0 (read in full): disable, or keep when the attached object reports via 0x142331080 (LOW)
}
ClearParachuteTimer();                                 // 0x142353760 (read in full)
if (NativeRecoveredPtr_0x4bd)
{
    NativeRecoveredPtr_0x4bd->VirtualStop_0x528(NativeRecoveredInt_0x25f4, 0, 0);   // vtable 0x528 (LOW)
    NativeRecoveredPtr_0x4bd = nullptr;
}
VirtualSetLocalState_0xb90(false);                     // vtable 0xb90 on this with false (LOW name)
FFortGlobals* Globals = GetFortGlobals();              // 0x141ed1800 (read in full: lazy static)
VirtualOnParachuteEnded_0xcf0(&Globals->NativeRecoveredParachuteTuning_0x6a0);   // vtable 0xcf0 (LOW)
if (NativeRecoveredPtr_0x177)                          // a tag-count container at this+0xbb8
{
    // 0x140744d30 = FGameplayTagCountContainer::UpdateTagCount(Tag, -1) (read in full): returns true when the tag count changes state.
    if (NativeRecoveredPtr_0x177->UpdateTagCount(Globals->NativeRecoveredTag_0x1f70, -1))
    {
        NativeRecoveredPtr_0x177->VirtualNotifyTagChanged_0x810(Globals->NativeRecoveredTag_0x1f70, false);   // vtable 0x810 (LOW)
    }
}
AFortPlayerControllerZone* ZoneController = NativeRecoveredPtr_0x4a;
if (ZoneController && ZoneController->IsA(AFortPlayerControllerZone::StaticClass()))   // 0x142a6ee20 (named)
{
    ZoneController->VirtualOnParachuteEnded_0x2268();  // vtable 0x2268 (LOW)
}
