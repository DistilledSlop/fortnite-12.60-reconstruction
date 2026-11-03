// Requires the complete legacy held-input interface/manager lifecycle. This
// fragment is intentionally not enabled by policy.py until that closure exists.
#include "CommonActionBindingData.h"
#include "CommonActionBindingRoute.h"
#include "UCommonActivatablePanel.h"
#include "UCommonActionWidget.h"
#include "UCommonUISubsystem.h"
#include "UCommonInputManager.h"
#include "UCommonGlobalInputHandler.h"
#include "UCommonInputSubsystem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/GameInstance.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/SlateApplication.h"

void UCommonButton::SetTriggeringInputAction(FDataTableRowHandle InputActionRow)
{
    // 14418a080 is full inequality, not a null-row test.
    if (TriggeringInputAction == InputActionRow) return;
    NativeUnbindTriggeringInputAction();
    TriggeringInputAction = InputActionRow;
    NativeBindTriggeringInputAction();
    NativeUpdateActionWidget();
}

void UCommonButton::NativeBindTriggeringInputAction()
{
    if (TriggeringInputAction.IsNull() || !NativeTriggeredInputAction.IsNull()) return;
    if (NativeTriggeringActionBinding < 0 || !CommonActionBindingData::Find(NativeTriggeringActionBinding))
    {
        FCommonActionBindingDescriptor Descriptor;
        Descriptor.Tag = NAME_None;
        Descriptor.Row = TriggeringInputAction;
        Descriptor.InputMode = static_cast<uint8>(InputModeOverride);
        Descriptor.Event = 0;
        Descriptor.Flag20 = bIsPersistentBinding;
        Descriptor.Flag21 = 1;
        Descriptor.Flag22 = 0;
        Descriptor.Click = FSimpleDelegate::CreateUObject(this, &UCommonButton::HandleButtonClicked);
        Descriptor.Progress = FCommonActionBindingProgress::CreateUObject(this, &UCommonButton::NativeOnActionProgress);
        NativeTriggeringActionBinding = NativeRegisterActionBinding(Descriptor);
    }
    if (NativeTriggeringActionBinding >= 0 && CommonActionBindingData::Find(NativeTriggeringActionBinding)) return;
    UGameInstance* Instance = GetGameInstance();
    UCommonUISubsystem* UI = Instance ? Instance->GetSubsystem<UCommonUISubsystem>() : nullptr;
    // Original1407abfb1 dereferences the subsystem even when lookup returned null.
    UCommonInputManager* Manager = UI->CommonInputManager;
    if (!Manager || !Manager->GlobalInputHandler) return;
    FCommonActionCommited Commit;
    FCommonActionProgressSingle Progress;
    FCommonActionCompleteSingle Complete;
    Commit.BindUFunction(this, TEXT("HandleTriggeringActionCommited"));
    Progress.BindUFunction(this, TEXT("NativeOnActionProgress"));
    Complete.BindUFunction(this, TEXT("NativeOnActionComplete"));
    int32 Priority = InputPriority;
    if (!Priority)
    {
        UObject* Outer = GetOuter();
        while (Outer && (Outer = Outer->GetOuter()))
        {
            UUserWidget* Widget = Cast<UUserWidget>(Outer);
            if (!Widget) break;
            if (UCommonActivatablePanel* Panel = Cast<UCommonActivatablePanel>(Widget))
            {
                Priority = static_cast<int32>(Panel->NativeGetGlobalInputPriorityFilter());
                break;
            }
            Outer = Widget->GetOuter();
        }
    }
    Manager->GlobalInputHandler->NativeRegisterGlobalAction(this, TriggeringInputAction, Priority, Commit, Complete, Progress);
}

void UCommonButton::NativeUnbindTriggeringInputAction()
{
    if (TriggeringInputAction.IsNull() || !NativeTriggeredInputAction.IsNull()) return;
    if (NativeTriggeringActionBinding >= 0 && CommonActionBindingData::Find(NativeTriggeringActionBinding))
        FCommonActionBindingRoute::Unregister(NativeTriggeringActionBinding);
    UGameInstance* Instance = GetGameInstance();
    UCommonUISubsystem* UI = Instance ? Instance->GetSubsystem<UCommonUISubsystem>() : nullptr;
    if (UI && UI->CommonInputManager && !TriggeringInputAction.IsNull())
        UI->CommonInputManager->GlobalInputHandler->NativeRemoveGlobalAction(this, TriggeringInputAction);
}

void UCommonButton::HandleTriggeringActionCommited(bool&)
{
    FSlateApplication::Get().PlaySound(NormalStyle.PressedSlateSound, 0);
    HandleButtonClicked();
}

void UCommonButton::NativeOnActionProgress(float HeldPercent)
{
    if (InputActionWidget && InputActionWidget->ProgressDynamicMaterial && !InputActionWidget->ProgressMaterialParam.IsNone())
        InputActionWidget->ProgressDynamicMaterial->SetScalarParameterValue(InputActionWidget->ProgressMaterialParam, HeldPercent);
    OnActionProgress(HeldPercent);
}

void UCommonButton::NativeOnActionComplete()
{
    if (InputActionWidget && InputActionWidget->ProgressDynamicMaterial && !InputActionWidget->ProgressMaterialParam.IsNone())
        InputActionWidget->ProgressDynamicMaterial->SetScalarParameterValue(InputActionWidget->ProgressMaterialParam, 0.0f);
    OnActionComplete();
}

void UCommonButton::NativeRegisterInputMethodListener()
{
    ULocalPlayer* Player = GetOwningLocalPlayer();
    UCommonInputSubsystem* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
    if (Input) Input->NativeInputMethodChanged.AddUObject(this, &UCommonButton::NativeHandleButtonInputMethodChanged);
}

void UCommonButton::NativeUnregisterInputMethodListener()
{
    ULocalPlayer* Player = GetOwningLocalPlayer();
    UCommonInputSubsystem* Input = Player ? Player->GetSubsystem<UCommonInputSubsystem>() : nullptr;
    if (Input) Input->NativeInputMethodChanged.RemoveAll(this);
}

void UCommonButton::NativeHandleButtonInputMethodChanged(ECommonInputType)
{ NativeUpdateActionWidget(); }

void UCommonButton::NativeConstruct()
{
    NativeBindTriggeringInputAction();
    NativeRegisterInputMethodListener();
    NativeUpdateActionWidget();
    UUserWidget::NativeConstruct();
}

void UCommonButton::NativeDestruct()
{
    Super::NativeDestruct();
    NativeUnbindTriggeringInputAction();
    NativeUnregisterInputMethodListener();
}
