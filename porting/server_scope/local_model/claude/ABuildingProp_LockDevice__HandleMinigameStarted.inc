// CONFIDENCE: MEDIUM. Minigame started: sets the minigame-active flag and sets channel 14 on the lock's static mesh to its default response.
// Original address: 0x14274ff80.
bMinigameActive = true;
if (StaticMeshComponent == nullptr)
{
    return;
}
StaticMeshComponent->SetCollisionResponseToChannel(static_cast<ECollisionChannel>(14));
