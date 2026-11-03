#include "CommonActionBindingData.h"
#include "CommonActionBindingRoute.h"
#include "UCommonInputManager.h"

// Original category1480d1df0, constructor1401c5610: defaultLog5, compileAll7.
COMMONUI_API DECLARE_LOG_CATEGORY_EXTERN(LogCommonUI, Log, All);
DEFINE_LOG_CATEGORY(LogCommonUI);

bool UCommonActivatablePanel::HasInputActionHandler(FDataTableRowHandle InputActionRow) const
{
    if (InputActionRow.IsNull())
    {
        UE_LOG(LogCommonUI, Warning, TEXT("Looking for an input action that is invalid!"));
        return false;
    }
    return NativeActionRecords.Contains(InputActionRow) || NativeActionBindingRows.Contains(InputActionRow);
}

bool UCommonActivatablePanel::NativeIsHoldingInputAction(const FDataTableRowHandle& Row) const
{
    if (!NativeInputManager.IsValid()) return false;
    return NativeInputManager.Get()->CurrentlyHeldActionInputHandler.GetObject() == this
        && Row == NativeHeldAction && NativeHeldActionDuration > 0.0f;
}

void UCommonActivatablePanel::NativeNotifyInputActionsChanged()
{
    if (NativeInputManager.IsValid())
    {
        auto* Manager = NativeInputManager.Get();
        if (Manager->ActivatablePanelStack.Contains(this) && bIsActive)
            Manager->NativePanelInputActionsChanged.Broadcast(); // original +78
    }
}

void UCommonActivatablePanel::NativeSetInputActionHandler(FDataTableRowHandle Row,
    FCommonPanelCommitNative Commit, FCommonActionBindingProgress Progress, UCommonPopupMenu* Popup)
{
    if (int32* Existing = NativeActionBindingRows.Find(Row))
        if (*Existing < 0 || !CommonActionBindingData::Find(*Existing)) NativeActionBindingRows.Remove(Row);
    if (NativeActionBindingRows.Contains(Row)) return;
    if (!Popup)
    {
        FCommonActionBindingDescriptor Descriptor;
        Descriptor.Tag = NAME_None;
        Descriptor.Row = Row;
        Descriptor.InputMode = 0;
        Descriptor.Event = 0;
        Descriptor.Flag20 = 0;
        Descriptor.Flag21 = 1;
        Descriptor.Flag22 = 1;
        Descriptor.Click = FSimpleDelegate::CreateWeakLambda(this, [Commit]() mutable {
            bool bPassThrough = false;
            Commit.ExecuteIfBound(bPassThrough); // source thunk 1407bc540/1407bd2f0
        });
        // Source does not forward legacy progress into the routed descriptor.
        const int32 Handle = NativeRegisterActionBinding(Descriptor);
        if (Handle >= 0 && CommonActionBindingData::Find(Handle)) NativeActionBindingRows.Add(Row, Handle);
        if (Handle >= 0 && CommonActionBindingData::Find(Handle)) return;
    }
    if (Row.IsNull() || NativeActionRecords.Contains(Row)) return;
    if (NativeIsHoldingInputAction(Row) && bIsActive) NativeClearHeldAction();
    FCommonPanelActionRecord Record;
    Record.NativeCommit = Commit;
    // Original creates and adds a progress delegate even for an empty dynamic wrapper.
    Record.NativeProgress.AddLambda([Progress](float Value) mutable { Progress.ExecuteIfBound(Value); });
    Record.HandlerData.InputActionRow = Row;
    Record.HandlerData.State = static_cast<EInputActionState>(0);
    Record.HandlerData.NativePopupMenu = Popup;
    NativeActionRecords.Add(Row, Record);
    NativeNotifyInputActionsChanged();
}

void UCommonActivatablePanel::RemoveInputActionHandler(FDataTableRowHandle InputActionRow)
{
    if (InputActionRow.IsNull()) return;
    auto* Legacy = NativeActionRecords.Find(InputActionRow);
    int32* Routed = NativeActionBindingRows.Find(InputActionRow);
    if (!Legacy && !Routed) return;
    if (NativeIsHoldingInputAction(InputActionRow) && bIsActive) NativeClearHeldAction();
    if (Legacy) NativeActionRecords.Remove(InputActionRow);
    else if (Routed)
    {
        FCommonActionBindingRoute::Unregister(*Routed);
        NativeActionBindingRows.Remove(InputActionRow);
    }
    NativeNotifyInputActionsChanged();
}

void UCommonActivatablePanel::NativeRemoveRoutedInputActionHandlers()
{
    for (auto& Pair : NativeActionBindingRows)
        FCommonActionBindingRoute::Unregister(Pair.Value);
    NativeActionBindingRows.Reset();
}

void UCommonActivatablePanel::RemoveAllInputActionHandlers()
{
    if (bIsActive) NativeClearHeldAction();
    NativeActionRecords.Reset();
    NativeNotifyInputActionsChanged();
    NativeRemoveRoutedInputActionHandlers();
}

void UCommonActivatablePanel::SetInputActionHandler(FDataTableRowHandle InputActionRow, FCommonActionCommited CommitedEvent)
{
    NativeSetInputActionHandler(InputActionRow,
        FCommonPanelCommitNative::CreateLambda([CommitedEvent](bool& Pass) mutable { CommitedEvent.ExecuteIfBound(Pass); }),
        FCommonActionBindingProgress::CreateLambda([](float) {}), nullptr);
}
void UCommonActivatablePanel::SetInputActionHandlerWithPopupMenu(FDataTableRowHandle InputActionRow,
    FCommonActionCommited CommitedEvent, UCommonPopupMenu* PopupMenu)
{
    NativeSetInputActionHandler(InputActionRow,
        FCommonPanelCommitNative::CreateLambda([CommitedEvent](bool& Pass) mutable { CommitedEvent.ExecuteIfBound(Pass); }),
        FCommonActionBindingProgress::CreateLambda([](float) {}), PopupMenu);
}
void UCommonActivatablePanel::SetInputActionHandlerWithProgress(FDataTableRowHandle InputActionRow,
    FCommonActionCommited CommitedEvent, FCommonActionProgressSingle ProgressEvent)
{
    NativeSetInputActionHandler(InputActionRow,
        FCommonPanelCommitNative::CreateLambda([CommitedEvent](bool& Pass) mutable { CommitedEvent.ExecuteIfBound(Pass); }),
        FCommonActionBindingProgress::CreateLambda([ProgressEvent](float Value) mutable { ProgressEvent.ExecuteIfBound(Value); }), nullptr);
}
void UCommonActivatablePanel::SetInputActionHandlerWithProgressPopupMenu(FDataTableRowHandle InputActionRow,
    FCommonActionCommited CommitedEvent, FCommonActionProgressSingle ProgressEvent, UCommonPopupMenu* PopupMenu)
{
    NativeSetInputActionHandler(InputActionRow,
        FCommonPanelCommitNative::CreateLambda([CommitedEvent](bool& Pass) mutable { CommitedEvent.ExecuteIfBound(Pass); }),
        FCommonActionBindingProgress::CreateLambda([ProgressEvent](float Value) mutable { ProgressEvent.ExecuteIfBound(Value); }), PopupMenu);
}
void UCommonActivatablePanel::AddInputActionHandler(UDataTable* DataTable, FName RowName, FCommonActionCommited CommitedEvent)
{ if (!DataTable) return; FDataTableRowHandle Row; Row.DataTable = DataTable; Row.RowName = RowName; SetInputActionHandler(Row, CommitedEvent); }
void UCommonActivatablePanel::AddInputActionHandlerWithPopup(UDataTable* DataTable, FName RowName,
    FCommonActionCommited CommitedEvent, UCommonPopupMenu* PopupMenu)
{ if (!DataTable) return; FDataTableRowHandle Row; Row.DataTable = DataTable; Row.RowName = RowName; SetInputActionHandlerWithPopupMenu(Row, CommitedEvent, PopupMenu); }
void UCommonActivatablePanel::AddInputActionHandlerWithProgress(UDataTable* DataTable, FName RowName,
    FCommonActionCommited CommitedEvent, FCommonActionProgressSingle ProgressEvent)
{ if (!DataTable) return; FDataTableRowHandle Row; Row.DataTable = DataTable; Row.RowName = RowName; SetInputActionHandlerWithProgress(Row, CommitedEvent, ProgressEvent); }
void UCommonActivatablePanel::AddInputActionHandlerWithProgressPopup(UDataTable* DataTable, FName RowName,
    FCommonActionCommited CommitedEvent, FCommonActionProgressSingle ProgressEvent, UCommonPopupMenu* PopupMenu)
{ if (!DataTable) return; FDataTableRowHandle Row; Row.DataTable = DataTable; Row.RowName = RowName; SetInputActionHandlerWithProgressPopupMenu(Row, CommitedEvent, ProgressEvent, PopupMenu); }
void UCommonActivatablePanel::AddInputActionNoHandler(UDataTable* DataTable, FName RowName)
{
    if (!DataTable) return;
    FDataTableRowHandle Row; Row.DataTable = DataTable; Row.RowName = RowName;
    NativeSetInputActionHandler(Row,
        FCommonPanelCommitNative::CreateLambda([](bool&) {}),
        FCommonActionBindingProgress::CreateLambda([](float) {}), nullptr);
}

void UCommonActivatablePanel::NativeDestruct()
{
    NativeActionRecords.Reset();
    NativeRemoveRoutedInputActionHandlers();
    OnWidgetActivated.Clear();
    OnWidgetDeactivated.Clear();
    // The inherited implementation performs the same guarded deactivation,
    // widget-router removal, and direct UUserWidget parent call.
    Super::NativeDestruct();
}
