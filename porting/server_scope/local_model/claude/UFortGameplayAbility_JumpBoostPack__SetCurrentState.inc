// CONFIDENCE: MEDIUM. Original exec 0x1428fe960; core 0x141639000.
// On an authoritative owner the server state is written first and the change only proceeds when the local state differs.
const EJumpBoostPackState PreviousState = CurrentState;
const bool bAuthoritativeOwner = IsValid(GetJetpackOwner()) && HasAuthorityForJetpack();

if (bAuthoritativeOwner)
{
	ServerCurrentState = InState;
	if (CurrentState == InState)
	{
		return;
	}
}

CurrentState = InState;
OnCurrentStateChanged(PreviousState, InState);
HandleCurrentStateChanged(PreviousState);
