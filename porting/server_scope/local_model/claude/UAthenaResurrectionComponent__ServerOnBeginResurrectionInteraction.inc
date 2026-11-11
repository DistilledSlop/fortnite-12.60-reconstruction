// CONFIDENCE: MEDIUM - validate (vslot 0x408) is Stub_ReturnTrue so the body is the implementation (0x141b53390); the spawn-machine query at +0x218 is called through a proposed name and its concrete class is not resolved.
// Original addresses: exec 0x14272b8c0; impl 0x141b53390 (vslot 0x410); validate 0x1406801d0
AFortPlayerControllerAthena* Controller = Cast<AFortPlayerControllerAthena>(InteractingPlayerController);
if (Controller == nullptr)
{
    return;
}
const FResurrectionInteractionContext Context = MakeResurrectionInteractionContext(Controller->GetInteractionPawn(), Controller, 2);
if (SpawnMachine != nullptr && SpawnMachine->InteractionQuery.CanInteract(Context))
{
    NativeBeginResurrectionInteraction(SpawnMachine, Controller);
}
