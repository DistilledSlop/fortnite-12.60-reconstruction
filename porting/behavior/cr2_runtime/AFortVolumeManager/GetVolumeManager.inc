// Original body: 0x141c7d4d0 (thunk 0x142b3c090). Offset evidence: no member access. Arguments per header_decl in Windows x64 positions: RCX = WorldContextObject, RDX = &OutVolumeManager (written with the resolved pointer), R8 = &OutIsValid (byte set to 1 when the resolved pointer is null, else 0 via SETZ).
// CONFIDENCE: HIGH
AFortVolumeManager* ResolvedManager = AFortVolumeManager::NativeResolveVolumeManager(WorldContextObject);
OutVolumeManager = ResolvedManager;
OutIsValid = static_cast<EVolumeValidityResult>(ResolvedManager == nullptr ? 1 : 0);
