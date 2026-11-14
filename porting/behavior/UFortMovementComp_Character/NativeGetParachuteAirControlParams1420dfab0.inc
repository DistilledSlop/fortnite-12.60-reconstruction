// Original address: 0x1420dfab0
// CONFIDENCE: high
if (AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
{
    if (Player->CurrentSkyTube != nullptr)
    {
        return ParachuteControlParamsSkydive;
    }
    if (Player->bIsInVortex)
    {
        return VortexParachuteControlParams;
    }
    if (Player->bInGliderRedeploy)
    {
        return ParachuteControlParamsGliderRedeploy;
    }
    return Player->bLocalIsSkydiving ? ParachuteControlParamsSkydive : ParachuteControlParamsJump;
}
return ParachuteControlParamsJump;
