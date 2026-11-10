// CONFIDENCE: MEDIUM. Minigame finished: clears the minigame-active flag and restores the saved collision response of channel 14 on the lock's static mesh.
// Original address: 0x14274ff60.
bMinigameActive = false;
if (StaticMeshComponent == nullptr)
{
    return;
}
StaticMeshComponent->SetCollisionResponseToChannel(static_cast<ECollisionChannel>(14), SavedCollisionResponse);
