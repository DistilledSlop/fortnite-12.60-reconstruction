// CONFIDENCE: high
// Original address: 0x141ea6f10
// 0x141ea6f10, primary-vtable byte offset 0x770.
const auto ResolveTeamActor = [](AActor* Actor) -> IFortTeamActorInterface*
{
    if (!Actor)
    {
        return nullptr;
    }
    if (Actor->GetInstigator())
    {
        if (!Cast<IFortTeamActorInterface>(Actor))
        {
            Actor = Actor->GetInstigator();
            if (!Actor)
            {
                return nullptr;
            }
        }
    }
    return Cast<IFortTeamActorInterface>(Actor);
};
IFortTeamActorInterface* Source = ResolveTeamActor(Requester);
IFortTeamActorInterface* Target = ResolveTeamActor(Candidate);
if (Source && Target)
{
    const uint8 TargetTeam = Target->NativeGetTeam163();
    const uint8 SourceTeam = Source->NativeGetTeam163();
    return TargetTeam == SourceTeam;
}
return false;
