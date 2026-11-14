// CONFIDENCE: HIGH. Complete original body; shared producer lifecycle remains separate.
// Full body of 0x142331a80. Uses reflected flags, shared native last-change time,
// and the exact Athena.LimitParachuteDeployRate CVar contract.
if (!bIsParachuteOpen || bLocalParachuteLockedOpen || bIsParachuteForcedOpen)
{
    return false;
}
if (GetLocalRole() == ROLE_Authority)
{
    if (bInGliderRedeploy) return false;
}
else if (GetLocalRole() == ROLE_AutonomousProxy)
{
    if (bLocalInGliderRedeploy) return false;
}
if (NativeLimitParachuteDeployRate148157cfc() != 0)
{
    UWorld* World = GetWorld();
    if (World && !(World->GetTimeSeconds() - NativeParachuteLastTime264c >= ParachuteCooldownToClose))
    {
        return false;
    }
}
return true;
