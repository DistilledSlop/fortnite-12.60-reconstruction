// CONFIDENCE: high; complete native source binding; see HANDOFF.md.
// Native 0x142443e60; see HANDOFF.md.
UFortQuestItem* Quest = nullptr;
UFortQuestObjectiveInfo* Objective = nullptr;
for (UFortQuestItem* Candidate : this->CurrentQuests) {
    if (Candidate && Candidate->ItemDefinition == Update.Quest) {
        Quest = Candidate;
        for (UFortQuestObjectiveInfo* Entry : Candidate->Objectives) {
            if (Entry->BackendName == Update.BackendName) {
                Objective = Entry;
                break;
            }
        }
        break;
    }
}
FFortMcpQuestObjectiveInfo* DefinitionObjective = nullptr;
if (Update.Quest) {
    for (FFortMcpQuestObjectiveInfo& Entry : Update.Quest->Objectives) {
        if (Entry.BackendName == Update.BackendName) {
            DefinitionObjective = &Entry;
            break;
        }
    }
}
if (!Objective || !DefinitionObjective) return;
Objective->AchievedCount = Update.StatValue;
Objective->QuestOwner = Owner;
Objective->AssistedPlayerState = Assisted;
UFortQuestItemDefinition* Definition = Update.Quest;
if (Definition && Definition->IsA<UFortFeatItemDefinition>()) {
    bool Complete = true;
    for (UFortQuestObjectiveInfo* Entry : Quest->Objectives) {
        if (Entry && Entry->AchievedCount < Entry->RequiredCount) {
            Complete = false;
            break;
        }
    }
    if (Complete) {
        this->FeatsCompletedThisSession.Add(static_cast<UFortFeatItemDefinition*>(Definition));
        if (GEngine) {
            if (UFortGameUserSettings* Settings = Cast<UFortGameUserSettings>(GEngine->GetGameUserSettings())) {
                Settings->bShowCareerTabBang = true;
            }
        }
    }
}
const bool DisplayStatus = NativeShouldDisplayObjectiveStatus(Objective, DefinitionObjective, Quest);
this->OnDisplayDynamicQuestUpdate.Broadcast(Objective, DisplayStatus, false);
