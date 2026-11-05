// CONFIDENCE: HIGH (source recovery; build/runtime pending)
if (!NativeIsWindEnabled() || GetNetMode() == NM_DedicatedServer)
{
    return;
}
for (TActorIterator<ABuildingSMActor> It(GetWorld()); It; ++It)
{
    ABuildingSMActor* const Building = *It;
    if (Building->bNeedsWindMaterialParameters)
    {
        FFortWindResponder Responder = NativeBuildWindResponder(Building);
        NativeRegisterWindResponder(Responder);
        Building->bNeedsWindMaterialParameters = false;
    }
}
