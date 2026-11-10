// CONFIDENCE: high; native body 0x141bd56a0, exec 0x142743b30.
if (PropActorToAttach != nullptr)
{
    PropActorToAttach->AttachToComponent(
        RootComponent,
        FAttachmentTransformRules(EAttachmentRule::SnapToTarget, EAttachmentRule::KeepWorld,
                                  EAttachmentRule::KeepWorld, true),
        NAME_None);
    if (AFortPlayerPawn* PlayerPawn = Cast<AFortPlayerPawn>(GetOwner()))
    {
        PlayerPawn->SetPossessedProp(this);
    }
}
