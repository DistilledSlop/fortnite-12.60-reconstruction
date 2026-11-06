// CONFIDENCE: high
// Original address: 0x1418804c0
if (!NameText.IsEmpty())
{
	TutorialData.DetailText = NameText;
	if (UWorld* World = GetWorld())
	{
		if (UFortTutorialContext* Context = World->GetSubsystem<UFortTutorialContext>())
		{
			Context->UpdateTutorialAnnouncement(TutorialData, true);
		}
	}
}
