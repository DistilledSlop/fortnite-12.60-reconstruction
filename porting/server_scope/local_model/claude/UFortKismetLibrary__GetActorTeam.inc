// CONFIDENCE: HIGH
// Native 0x14296bb30 -> 0x141eefb60: returns the team byte of the actor's team interface, or of its instigator when the
// actor does not implement it. Returns 0 when there is no actor or no team interface is found.
if (!Actor)
{
    return 0;
}
IFortTeamActorInterface* TeamInterface = static_cast<IFortTeamActorInterface*>(Actor->GetInterfaceAddress(UFortTeamActorInterface::StaticClass()));
if (!TeamInterface)
{
    const AActor* Instigator = Actor->GetInstigator();
    if (!Instigator)
    {
        return 0;
    }
    TeamInterface = static_cast<IFortTeamActorInterface*>(Instigator->GetInterfaceAddress(UFortTeamActorInterface::StaticClass()));
    if (!TeamInterface)
    {
        return 0;
    }
}
return TeamInterface->NativeGetTeam163();
