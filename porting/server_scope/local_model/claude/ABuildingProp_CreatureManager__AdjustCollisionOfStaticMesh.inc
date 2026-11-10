// CONFIDENCE: LOW. Rewrites the collision responses of a static mesh component. bIgnore selects a fixed list of creature-related channels set to Ignore; otherwise channels 0..32 are restored from the class default object's primitive. The result is applied to the mesh.
// Original addresses: exec thunk 0x14274a920; direct helper 0x141bfbc10 (inlined below).
// The thunk reads bIgnore before Mesh; the reflection lists Mesh first. The body uses names only.
if (Mesh == nullptr)
{
    return;
}
FCollisionResponseContainer Responses;
if (bIgnore)
{
    static const int32 IgnoredChannels[] = { 2, 3, 4, 5, 6, 14, 16, 17, 18, 20, 21, 22, 25, 27, 29, 30 };
    for (int32 Channel : IgnoredChannels)
    {
        Responses.SetResponse(static_cast<ECollisionChannel>(Channel), ECR_Ignore);
    }
}
else
{
    UPrimitiveComponent* Template = GetClass()->GetDefaultObject<UPrimitiveComponent>();
    if (Template != nullptr)
    {
        for (int32 Channel = 0; Channel <= 32; ++Channel)
        {
            Responses.SetResponse(static_cast<ECollisionChannel>(Channel), Template->GetCollisionResponseToChannel(static_cast<ECollisionChannel>(Channel)));
        }
    }
}
Mesh->SetCollisionResponseToChannels(Responses);
