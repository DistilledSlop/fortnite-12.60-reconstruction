// CONFIDENCE: HIGH. Complete original body.
// Original 0x1426a2720. Audio+84 mask1 is bRegistered (engine header order).
// Pinned bool_bits: +85 mask80 bAutoActivate, +210 mask2
// bStopWhenOwnerDestroyed, +212 mask8 bAutoManageAttachment.
if (Audio && !Audio->IsRegistered())
{
    Audio->bAutoActivate = false;
    Audio->bStopWhenOwnerDestroyed = false;
    Audio->bAutoManageAttachment = true;
    Audio->AutoAttachParent = GetRootComponent()->GetAttachParent();
    Audio->AutoAttachSocketName = GetRootComponent()->GetAttachSocketName();
    Audio->AutoAttachLocationRule = EAttachmentRule::KeepRelative;
    Audio->AutoAttachRotationRule = EAttachmentRule::KeepRelative;
    Audio->AutoAttachScaleRule = EAttachmentRule::KeepRelative;
    Audio->RegisterComponent(); // 0x1440c2bb0 owner->GetWorld, RegisterComponentWithWorld
    if (ItemWrapModifier)
    {
        ItemWrapModifier->NotifyAudioComponentAdded(Audio); // 0x14277bcf0 ProcessEvent
    }
}
