// Original manager vtable +0x150 -> 14514f610. Requires Engine/GameInstance.h.
// Its Outer is the director subsystem; helper14460bc60 casts that subsystem's Outer.
UWorld* UFortUIManager::GetWorld() const
{
    UGameInstance* GameInstance = Cast<UGameInstance>(GetOuter()->GetOuter());
    return GameInstance->UGameInstance::GetWorld();
}
