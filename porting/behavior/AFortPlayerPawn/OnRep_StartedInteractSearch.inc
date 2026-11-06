// CONFIDENCE: HIGH
if (bStartedInteractSearch)
{
    OnStartedInteractSearch.Broadcast();
}
else
{
    OnEndedInteractSearch.Broadcast();
}
