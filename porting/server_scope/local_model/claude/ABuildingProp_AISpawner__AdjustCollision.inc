// CONFIDENCE: MEDIUM. Sets the collision response of the given component: copy the owner actor's root responses, or when bIgnore is set make a fixed channel list ignore.
// Original address: 0x14274a920 (exec). Native body: 0x141bfbc10 (shared with ABuildingProp_CreatureManager::AdjustCollisionOfStaticMesh).
if (Collision == nullptr)
{
    return;
}
FCollisionResponseContainer Responses = FCollisionResponseContainer::GetDefault();   // native 0x144471420
if (!bIgnore)
{
    UPrimitiveComponent* OwnerRoot = OwnerActor->GetRootComponent();                // OwnerActor is the object at +0x10
    if (OwnerRoot != nullptr)
    {
        for (int32 Channel = 0; Channel < 32; ++Channel)
        {
            Responses.SetResponse(Channel, OwnerRoot->GetCollisionResponseToChannel(Channel));   // native 0x1444889d0 per channel
        }
    }
}
else
{
    static const int32 IgnoredChannels[] = { 3, 4, 14, 16, 17, 18, 20, 21, 22, 25, 27, 29, 30, 2, 5, 6 };
    for (int32 Channel : IgnoredChannels)
    {
        Responses.SetResponse(Channel, ECR_Ignore);   // value per the 2-argument call in the native body (LOW: the third argument is not visible in the decompile)
    }
}
Collision->SetCollisionResponseContainer(Responses);   // virtual slot 0x810 on the collision component
