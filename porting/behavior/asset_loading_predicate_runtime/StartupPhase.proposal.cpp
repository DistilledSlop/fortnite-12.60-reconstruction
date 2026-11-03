// Source proposal for 0x141ed7750. Requires the original UFortAssetManager
// native startup-phase field/getter and EFortStartupPhase declaration to be
// restored first. GetStartupPhase names the proven +0x6cd getter (0x141dfb870).
// Do not substitute the checked/fatal asset-manager accessor here.
EFortStartupPhase GetFortStartupPhase()
{
    if (GEngine)
    {
        if (const UFortAssetManager* Manager = Cast<UFortAssetManager>(GEngine->AssetManager))
        {
            return Manager->GetStartupPhase();
        }
    }
    return EFortStartupPhase::InitializingEngine;
}
