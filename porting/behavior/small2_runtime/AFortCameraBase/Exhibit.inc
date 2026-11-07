// Behaviour snippet: AFortCameraBase::Exhibit (body only; spliced into the generated member function).
// Original address: 0x141ca4480
// Evidence: Compare pointer RDX with [RCX+0x7a0] = reflected ExhibitActor (Object).
// Evidence: Store qword [RCX+0x7a0] = ExhibitActor; byte [RCX+0x7a8]=1 is reflected bitfield bExhibitActorChanged (mask 1).
// CONFIDENCE: HIGH
if (ExhibitActor == TargetActor)
{
    return;
}
ExhibitActor = TargetActor;
bExhibitActorChanged = true;
