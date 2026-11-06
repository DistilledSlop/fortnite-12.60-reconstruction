// CONFIDENCE: high
// Native 0x142321e70; wrapper 0x142a1c470.
for (const FFortActiveMontageDecisionWindow& Window : DecisionWindowStack)
{
    if (Window.DecisionWindow)
    {
        UFortAnimNotifyState_AbilityDecisionWindow* DecisionWindow = Window.DecisionWindow;
        return DecisionWindow->ComboCounter;
    }
}
return 0;
