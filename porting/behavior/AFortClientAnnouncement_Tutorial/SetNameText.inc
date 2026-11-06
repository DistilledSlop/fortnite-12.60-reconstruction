// CONFIDENCE: high
// Original address: 0x141880bb0
if (!NameText.IsEmpty())
{
	TutorialData.NameText = NameText;
	if (UWorld* World = GetWorld())
	{
		if (UFortTutorialContext* Context = World->GetSubsystem<UFortTutorialContext>())
		{
			Context->UpdateTutorialAnnouncement(TutorialData, true);
		}
	}
}
