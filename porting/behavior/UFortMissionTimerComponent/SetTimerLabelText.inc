// CONFIDENCE: high; captured native flow and typed dependency behavior.
{
    AActor* Owner = GetOwner();
    if (Owner && Owner->GetLocalRole() == ROLE_Authority && !TimerLabelText.EqualTo(NewLabelText))
    {
        Owner->ForceNetUpdate();
        TimerLabelText = NewLabelText;
    }
}
