// CONFIDENCE: high; 0x141c77c90. Component lookup is virtual and remains so through FindComponentByClass.
if (!Volume->bNeverAllowSaving)
{
    return 1;
}
UActorComponent* Component = Volume->FindComponentByClass(ULevelSaveComponent::StaticClass());
if (!Component)
{
    return 5;
}
UFortLevelSaveComponent* FortComponent = Cast<UFortLevelSaveComponent>(Component);
if (!FortComponent)
{
    return 4;
}
if (FortComponent->bCuratedHub)
{
    return 5;
}
return FortComponent->bCurated ? 3 : 2;
