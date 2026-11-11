// CONFIDENCE: MEDIUM - validate (vslot 0x3f8) is Stub_ReturnTrue; implementation 0x141b53420 read in full; the interrupt native 0x141bef340 is declared only.
// Original addresses: exec 0x14272b980; impl 0x141b53420 (vslot 0x400); validate 0x1406801d0
AFortPlayerControllerAthena* Controller = Cast<AFortPlayerControllerAthena>(InteractingPlayerController);
if (Controller != nullptr && SpawnMachine != nullptr)
{
    NativeInterruptResurrectionInteraction(SpawnMachine, Controller);
}
