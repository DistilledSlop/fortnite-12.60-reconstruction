// CONFIDENCE: MEDIUM - exec 0x14275e9e0 steps ItemDef (8 bytes) and InTrapLevel (4 bytes), then calls helper 0x141c4f360.
// Original addresses: 0x14275e9e0 (exec thunk) ; 0x141c4f360 (direct helper)
// The trap data and both level fields are written, then the volume manager of the game state is notified (only when one exists).
TrapData = ItemDef;
TrapLevel = InTrapLevel;
OriginalTrapLevel = InTrapLevel;
if (AFortVolumeManager* VolumeManager = GetVolumeManagerForActor(this))
{
    NotifyTrapSpawned(VolumeManager, this);
}
