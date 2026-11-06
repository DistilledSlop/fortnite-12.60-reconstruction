// Proposal only: apply via the source policy / generator, after verifying UE interface support.
// Interface declaration, IFortVolumeAccessor:
// virtual AFortVolume* NativeGetVolume() = 0;
//
// Each implementing class must also inherit public IFortVolumeAccessor and declare:
// virtual AFortVolume* NativeGetVolume() override;
//
// Proposed method bodies, one per class:
AFortVolume* AFortMinigame::NativeGetVolume()
{
    return Volume;
}

AFortVolume* AFortMinigameSettingsBuilding::NativeGetVolume()
{
    return SettingsVolume;
}

AFortVolume* AFortVolume::NativeGetVolume()
{
    return this;
}
