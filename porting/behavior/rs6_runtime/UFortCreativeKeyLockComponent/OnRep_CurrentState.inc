// Behaviour snippet: UFortCreativeKeyLockComponent::OnRep_CurrentState (body only; spliced into the generated member function).
// Original address: 0x141d8bd20
// Offset evidence: CurrentState enum byte at +0x138 is broadcast on OnLockStateChanged (+0x118, exact). Then state 0 broadcasts OnLocked (+0x108, exact) and state 1 broadcasts OnUnlocked (+0xf8, exact); other states do nothing. Both event delegates take no parameters.
// CONFIDENCE: HIGH
OnLockStateChanged.Broadcast(CurrentState);
if (static_cast<uint8>(CurrentState) == 0)
{
	OnLocked.Broadcast();
}
else if (static_cast<uint8>(CurrentState) == 1)
{
	OnUnlocked.Broadcast();
}
