// Uncounted behavioral draft. Layout/ABI dependencies are documented separately.
#include "CoreMinimal.h"
#include "FFortUpdatedObjectiveStat.h"
#include "FFortMcpQuestObjectiveInfo.h"
#include "UFortQuestItem.h"
#include "UFortQuestItemDefinition.h"
#include "UFortQuestObjectiveInfo.h"
#include "UFortQuestManager.h"
#include "UFortFeatItemDefinition.h"

namespace quest_assist87 {
// Native's out argument is a stack 8-byte slot. Exact return/out type and set
// ABI are unresolved; this call performs work and cannot be stubbed.
extern void add_140d75820(void* set_b58, void* out_slot, UFortQuestItemDefinition* const* key, int32 zero);
extern void* singleton_141e9a5f0();
extern uint8 display_status_1424511c0(UFortQuestObjectiveInfo*, FFortMcpQuestObjectiveInfo*, UFortQuestItem*);

void update_142443e60(UFortQuestManager* manager, const FFortUpdatedObjectiveStat& update,
                      AFortPlayerState* owner, AFortPlayerState* assisted)
{
    UFortQuestItem* quest = nullptr;
    UFortQuestObjectiveInfo* objective = nullptr;
    for (UFortQuestItem* candidate : manager->CurrentQuests) {
        if (candidate && candidate->ItemDefinition == update.Quest) {
            quest = candidate;
            // Native dereferences each element without a null guard here.
            for (UFortQuestObjectiveInfo* entry : candidate->Objectives) {
                if (entry->BackendName == update.BackendName) {
                    objective = entry;
                    break;
                }
            }
            break; // first matching quest even when it contains no matching objective
        }
    }
    FFortMcpQuestObjectiveInfo* definition_objective = nullptr;
    if (update.Quest) {
        for (FFortMcpQuestObjectiveInfo& entry : update.Quest->Objectives) {
            if (entry.BackendName == update.BackendName) {
                definition_objective = &entry;
                break;
            }
        }
    }
    if (!objective || !definition_objective) return;
    objective->AchievedCount = update.StatValue;
    objective->QuestOwner = owner;
    objective->AssistedPlayerState = assisted;
    // Re-read the quest pointer after the three writes, as in the original.
    UFortQuestItemDefinition* definition = update.Quest;
    if (definition && definition->IsA<UFortFeatItemDefinition>()) {
        bool incomplete = false;
        for (UFortQuestObjectiveInfo* entry : quest->Objectives) {
            if (entry && entry->AchievedCount < entry->RequiredCount) {
                incomplete = true;
                break;
            }
        }
        if (!incomplete) {
            uint64 output_slot; // written by callee; result is not subsequently used
            auto* set_b58 = reinterpret_cast<uint8*>(manager) + 0xb58;
            add_140d75820(set_b58, &output_slot, &definition, 0);
            if (void* singleton = singleton_141e9a5f0())
                *(reinterpret_cast<uint8*>(singleton) + 0x184) = 1;
        }
    }
    const uint8 display_status = display_status_1424511c0(objective, definition_objective, quest);
    manager->OnDisplayDynamicQuestUpdate.Broadcast(objective, display_status, false);
}
}
