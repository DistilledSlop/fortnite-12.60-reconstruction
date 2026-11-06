// CONFIDENCE: high
// Original address: 0x141881270
if (!SystemText.IsEmpty())
{
	TutorialData.SystemText = SystemText;
	if (UWorld* World = GetWorld())
	{
		if (UFortTutorialContext* Context = World->GetSubsystem<UFortTutorialContext>())
		{
			Context->UpdateTutorialAnnouncement(TutorialData, true);
		}
	}
}
