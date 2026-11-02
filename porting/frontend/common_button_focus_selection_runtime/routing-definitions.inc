#include "UCommonActivatablePanel.h"
#include "UCommonInputManager.h"
#include "UCommonUISubsystem.h"
#include "Engine/GameInstance.h"

void UCommonButton::NativeDispatchTriggeredAction()
{
    if (NativeTriggeredInputAction.IsNull()) return;
    UGameInstance* GameInstance = GetGameInstance();
    UCommonUISubsystem* Subsystem = GameInstance ? GameInstance->GetSubsystem<UCommonUISubsystem>() : nullptr;
    // The original dereferences the subsystem even when lookup returned null.
    UCommonInputManager* Manager = Subsystem->CommonInputManager;
    if (!Manager) return;
    for (int32 Index = Manager->ActivatablePanelStack.Num() - 1; Index >= 0; --Index)
    {
        UCommonActivatablePanel* Panel = Manager->ActivatablePanelStack[Index];
        if (!Panel) continue;
        int32 Slot = INDEX_NONE;
        FCommonPanelActionRecord* Record = Panel->NativeActionRecords.FindWithSlot(NativeTriggeredInputAction, Slot);
        if (Panel->IsActivated() && Record)
        {
            // The original reuses the low byte of the sparse-map index as the out argument.
            bool bPassThrough;
            const uint8 SlotByte = static_cast<uint8>(Slot);
            static_assert(sizeof(bPassThrough) == sizeof(SlotByte), "original one-byte bool ABI required");
            FMemory::Memcpy(&bPassThrough, &SlotByte, sizeof(SlotByte));
            Record->NativeCommit.ExecuteIfBound(bPassThrough);
            uint8 ResultByte;
            FMemory::Memcpy(&ResultByte, &bPassThrough, sizeof(ResultByte));
            (void)(ResultByte != 0); // Original discards the out value.
            break;
        }
    }
}
