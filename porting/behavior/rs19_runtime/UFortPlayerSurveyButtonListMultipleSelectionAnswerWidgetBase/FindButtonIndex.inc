// Behaviour snippet: UFortPlayerSurveyButtonListMultipleSelectionAnswerWidgetBase::FindButtonIndex (body only).
// Original body address: 0x144f9de30 (thunk 0x1452a8270, group B leaf).
// Offset evidence: ButtonWidgets = reflected TArray at +0x318 (Num at +0x320); element stride 0x18; element +0x0 = ButtonWidget (UCommonButton*, reflected in FFortPlayerSurveyButtonListMultipleSelectionAnswerWidgetBaseInternalButtonData).
// Asm: linear scan comparing qword [element+0] with Button; returns (element - Data) / 0x18 on match, -1 when not found or empty.
// CONFIDENCE: HIGH
for (int32 Index = 0; Index < ButtonWidgets.Num(); ++Index)
{
	if (ButtonWidgets[Index].ButtonWidget == Button)
	{
		return Index;
	}
}
return -1;
