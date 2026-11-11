// CONFIDENCE: MEDIUM. Original exec 0x1428fe320; core 0x14162a180.
// Boost spends jetpack fuel from the jetpack owner; leaving Hovering stamps the avatar sequence number.
if (CurrentState == EJumpBoostPackState::Boost)
{
	APawn* JetpackOwner = GetJetpackOwner();
	if (IsValid(JetpackOwner))
	{
		if (UFortJetpackComponent* Jetpack = JetpackOwner->FindComponentByClass<UFortJetpackComponent>())
		{
			static const FGameplayTag FuelTag = FGameplayTag::RequestGameplayTag(FName(TEXT("Fuel")));
			const float FuelCost = Jetpack->GetFuelDrainRate();
			JetpackOwner->ModifyFuelTagValue(FuelTag, -FuelCost);
		}
	}
}

if (InPreviousState == EJumpBoostPackState::Hovering)
{
	LastHoverSequence = GetAvatarSequenceNumber();
}
