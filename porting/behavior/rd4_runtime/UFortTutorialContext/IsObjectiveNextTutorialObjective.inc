// Behaviour snippet: UFortTutorialContext::IsObjectiveNextTutorialObjective (body only; spliced into the generated member function).
// Original address: 0x144d133d0 (thunk 0x14530af00)
// Offset evidence: Quest->Objectives is the reflected TArray<UFortQuestObjectiveInfo*> at +0xe8 (Num at +0xf0; UFortQuestItem.h). Objective->BackendName is the reflected FName at +0x28 (UFortQuestObjectiveInfo.h). IsInProgress() is declared const bool (UFortQuestObjectiveInfo.h; body 0x142447090).
// Control flow: a NAME_None parameter returns false; the first objective for which IsInProgress() is true decides the result by comparing its BackendName with the parameter; no in-progress objective returns false.
// CONFIDENCE: HIGH
if (TutorialObjectiveName == NAME_None)
{
	return false;
}

for (int32 Index = 0; Index < Quest->Objectives.Num(); ++Index)
{
	UFortQuestObjectiveInfo* Objective = Quest->Objectives[Index];
	if (Objective->IsInProgress())
	{
		return Objective->BackendName == TutorialObjectiveName;
	}
}

return false;
