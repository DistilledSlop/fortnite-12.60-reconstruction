// CONFIDENCE: HIGH
// Native 0x14296ba60 -> 0x141eefae0: the team byte comes from the actor's team interface, or from its instigator when
// the actor does not implement it. Team values 0..2 are not PvP teams; values 3 and up map to PvP team index (value - 3).
if (!Actor)
{
    return false;
}
IFortTeamActorInterface* TeamInterface = static_cast<IFortTeamActorInterface*>(Actor->GetInterfaceAddress(UFortTeamActorInterface::StaticClass()));
if (!TeamInterface)
{
    const AActor* Instigator = Actor->GetInstigator();
    if (!Instigator)
    {
        return false;
    }
    TeamInterface = static_cast<IFortTeamActorInterface*>(Instigator->GetInterfaceAddress(UFortTeamActorInterface::StaticClass()));
    if (!TeamInterface)
    {
        return false;
    }
}
const uint8 Team = TeamInterface->NativeGetTeam163();
if (Team < 3)
{
    return false;
}
TeamIndex = Team - 3;
return true;
