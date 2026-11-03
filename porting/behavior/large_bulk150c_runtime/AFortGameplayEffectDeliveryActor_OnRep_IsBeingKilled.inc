// Native vtable +0x6c0, 0x142697870. CONFIDENCE: high
SetActorEnableCollision(false);
SetActorTickEnabled(false);
if (bSetHiddenOnKill)
{
    SetActorHiddenInGame(true);
}
GetWorldTimerManager().ClearTimer(NativeTouchTimer);
if (CollisionComponent)
{
    CollisionComponent->OnComponentHit.RemoveAll(this);
    CollisionComponent->OnComponentBeginOverlap.RemoveAll(this);
    CollisionComponent->OnComponentEndOverlap.RemoveAll(this);
}
