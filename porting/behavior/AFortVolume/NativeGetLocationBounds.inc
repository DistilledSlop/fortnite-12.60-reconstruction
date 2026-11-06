// CONFIDENCE: HIGH
// 0x141c75610: the original leaves the output untouched when the component is absent.
UPawnOverlapComponent* Component = BoundsComponent;
if (Component)
{
    OutBounds = Component->Bounds;
}
else
{
    UE_LOG(LogFort, Error, TEXT("AFortVolume::GetBounds - The static mesh should be initialized for: %s"), *GetName());
}
