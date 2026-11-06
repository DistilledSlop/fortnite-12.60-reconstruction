// CONFIDENCE: high
// Original address: 0x141881480
if (!NameText.IsEmpty())
{
	TutorialData.TitleText = NameText;
	if (UWorld* World = GetWorld())
	{
		if (UFortTutorialContext* Context = World->GetSubsystem<UFortTutorialContext>())
		{
			Context->UpdateTutorialAnnouncement(TutorialData, true);
		}
	}
}
