// Native vtable +0x650, 0x1426a5f50. CONFIDENCE: high
if (GetLocalRole() != ROLE_Authority || IsBeingKilled())
{
    return;
}
FTimerManager& Manager = GetWorldTimerManager();
if (Manager.IsTimerActive(NativeKillTimer))
{
    return;
}
bIsBeingKilled = true;
OnRep_IsBeingKilled();
Manager.SetTimer(NativeKillTimer, this, &AFortGameplayEffectDeliveryActor::KillTimerExpired, LifespanAfterKill, false);
