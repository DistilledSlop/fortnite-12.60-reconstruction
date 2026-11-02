#include "CommonActionBindingRoute.h"
#include "UCommonUIActionRouter.h"
#include "UCommonActivatableWidget.h"
#include "UCommonInputSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Engine/GameInstance.h"
#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "Slate/SObjectWidget.h"
#include "Widgets/SViewport.h"
#include "FastUpdate/WidgetProxy.h"
#include "HAL/IConsoleManager.h"

namespace
{
// 1401c4ee0 registers the original backing byte 1480d1c81.
bool GCommonUIAlwaysShowCursor = false;
FAutoConsoleVariableRef CVarCommonUIAlwaysShowCursor(TEXT("CommonUI.AlwaysShowCursor"),
    GCommonUIAlwaysShowCursor, TEXT(""), ECVF_Default);

int32 LocalPlayerIndex(const ULocalPlayer* Player)
{
    return Player->GetGameInstance()->GetLocalPlayers().IndexOfByKey(Player);
}

// 1407be930: the first activatable parent owned by another local player ends
// the search. The supplied Slate widget itself is deliberately not inspected.
UCommonActivatableWidget* FindActivatableAncestor(TSharedPtr<SWidget> SlateWidget, const ULocalPlayer* Player)
{
    while (SlateWidget.IsValid())
    {
        SlateWidget = SlateWidget->GetParentWidget();
        if (SlateWidget.IsValid() && SlateWidget->GetType() == FName(TEXT("SObjectWidget")))
        {
            auto* Activatable = Cast<UCommonActivatableWidget>(StaticCastSharedPtr<SObjectWidget>(SlateWidget)->GetWidgetObject());
            if (Activatable) return Activatable->GetOwningLocalPlayer() == Player ? Activatable : nullptr;
        }
    }
    return nullptr;
}
}

bool RouteShouldShowCursorForTouch(UGameViewportClient* ViewportClient)
{
    bool bShow = FSlateApplication::Get().IsFakingTouchEvents();
    if (ViewportClient) bShow |= ViewportClient->GetUseMouseForTouch();
    return GCommonUIAlwaysShowCursor || bShow;
}

FCommonActionBindingRoute::FCommonActionBindingRoute(UCommonUIActionRouter* InOwner) : Owner(InOwner) {}
FCommonActionRouteOwnerRecord::FCommonActionRouteOwnerRecord(UWidget* InWidget) : Widget(InWidget) {}
TSharedRef<FCommonActionBindingRoute> FCommonActionBindingRoute::Create(UCommonUIActionRouter* InOwner)
{
    auto Result = MakeShared<FCommonActionBindingRoute>(InOwner);
    Result->WeakSelf = Result;
    return Result;
}
bool FCommonActionBindingRoute::IsRoutingActive() const { return true; }
void FCommonActionBindingRoute::NotifyBindingsIfActive()
{
    if (IsRoutingActive()) Owner.Get()->NativeActionBindingsChanged.Broadcast();
}
void FCommonActionBindingRoute::Associate(FCommonActionBindingRecord& Record)
{
    if (BindingHandles.Contains(Record.Id)) return;
    BindingHandles.Add(Record.Id);
    {
        const auto Self = WeakSelf.Pin();
        Record.Route = Self;
    }
    NotifyBindingsIfActive();
}
void FCommonActionBindingRoute::Disassociate(int32 Id)
{
    if (BindingHandles.Remove(Id) <= 0) return;
    if (auto* Record = CommonActionBindingData::Find(Id)) Record->Route.Reset();
    NotifyBindingsIfActive();
}
void FCommonActionBindingRoute::Disassociate(const TArray<int32>& Handles)
{
    for (int32 Handle : Handles) Disassociate(Handle);
}
void FCommonActionBindingRoute::Unregister(int32& Handle)
{
    if (auto* Record = CommonActionBindingData::Find(Handle))
    {
        if (Record->Route.IsValid())
        {
            const auto Route = Record->Route.Pin();
            Route->Disassociate(Handle);
        }
    }
    CommonActionBindingData::Remove(Handle);
    Handle = INDEX_NONE;
}

FCommonActionRouteNode::FCommonActionRouteNode(UCommonUIActionRouter* InOwner, UCommonActivatableWidget* InWidget)
    : FCommonActionBindingRoute(InOwner), Widget(InWidget) {}
FCommonActionRouteNode::~FCommonActionRouteNode()
{
    // 140799860: the node vtable is active during this continuation.
    if (bRoutingActive && Owner.IsValid()) HandleWidgetDeactivated();
}
bool FCommonActionRouteNode::IsRoutingActive() const
{
    return bRoutingActive && Widget.IsValid() && Widget.Get()->IsActivated();
}
UCommonActivatableWidget* FCommonActionRouteNode::GetWidget() const { return Widget.Get(); }
void FCommonActionRouteNode::Initialize()
{
    const auto Self = StaticCastSharedRef<FCommonActionRouteNode>(WeakSelf.Pin().ToSharedRef());
    Widget.Get()->NativeWidgetActivated.AddSP(Self, &FCommonActionRouteNode::HandleWidgetActivated);
    Widget.Get()->NativeWidgetDeactivated.AddSP(Self, &FCommonActionRouteNode::HandleWidgetDeactivated);
    if (Widget.Get()->IsActivated()) HandleWidgetActivated();
}
TSharedRef<FCommonActionRouteNode> FCommonActionRouteNode::AddChild(UCommonActivatableWidget* ChildWidget)
{
    const auto Self = StaticCastSharedRef<FCommonActionRouteNode>(WeakSelf.Pin().ToSharedRef());
    ChildWidget->NativeWidgetReleased.AddSP(Self, &FCommonActionRouteNode::RemoveChildForWidget, ChildWidget);
    auto Child = MakeShared<FCommonActionRouteNode>(Owner.Get(), ChildWidget);
    Child->WeakSelf = Child;
    Child->Parent = Self;
    Child->Initialize();
    Children.Add(Child);
    return Child;
}
void FCommonActionRouteNode::RemoveChildForWidget(UCommonActivatableWidget* ChildWidget)
{
    ChildWidget->NativeWidgetReleased.RemoveAll(this);
    const int32 Index = Children.IndexOfByPredicate([ChildWidget](const TSharedPtr<FCommonActionRouteNode>& Child)
    {
        return Child->Widget.Get() == ChildWidget;
    });
    if (Index != INDEX_NONE) Children.RemoveAt(Index, 1, true);
}
void FCommonActionRouteNode::SetRoutingActive(bool bActive)
{
    if (bActive == bRoutingActive) return;
    if (bActive && (!Widget.IsValid() || !Widget.Get()->IsActivated())) return;
    bRoutingActive = bActive;
    // The original captures both ranges once, without a retained array snapshot.
    auto* Processor = InputProcessors.GetData();
    auto* const ProcessorEnd = Processor + InputProcessors.Num();
    for (; Processor != ProcessorEnd; ++Processor)
    {
        if (bActive) FSlateApplication::Get().RegisterInputPreProcessor(Processor->InputProcessor, Processor->Index);
        else FSlateApplication::Get().UnregisterInputPreProcessor(Processor->InputProcessor);
    }
    auto* Child = Children.GetData();
    auto* const ChildEnd = Child + Children.Num();
    for (; Child != ChildEnd; ++Child)
    {
        const auto RetainedChild = *Child;
        RetainedChild->SetRoutingActive(bActive);
    }
}
void FCommonActionRouteNode::AddInputPreProcessor(const TSharedPtr<IInputProcessor>& Processor, int32 Index)
{
    InputProcessors.Add({Index, Processor});
    if (IsRoutingActive()) FSlateApplication::Get().RegisterInputPreProcessor(Processor, Index);
}
void FCommonActionRouteNode::AddRoutedWidget(UWidget* InWidget)
{
    RoutedWidgets.AddUnique(TWeakObjectPtr<UWidget>(InWidget));
}
bool FCommonActionRouteNode::SupportsFocusThroughParents() const
{
    bool bSupports = Widget.IsValid() && Widget.Get()->bSupportsActivationFocus;
    auto Ancestor = Parent.Pin();
    while (Ancestor.IsValid() && bSupports)
    {
        bSupports = Ancestor->Widget.IsValid() && Ancestor->Widget.Get()->bSupportsActivationFocus;
        Ancestor = Ancestor->Parent.Pin();
    }
    return bSupports;
}
TSharedPtr<FCommonActionRouteRoot> FCommonActionRouteNode::GetRoot()
{
    const auto ParentNode = Parent.Pin();
    if (ParentNode.IsValid()) return ParentNode->GetRoot();
    return StaticCastSharedPtr<FCommonActionRouteRoot>(WeakSelf.Pin());
}
void FCommonActionRouteNode::HandleWidgetActivated()
{
    OnWidgetActivated.ExecuteIfBound();
    const auto ParentNode = Parent.Pin();
    if (ParentNode.IsValid() && ParentNode->IsRoutingActive())
    {
        SetRoutingActive(true);
        if (SupportsFocusThroughParents())
        {
            auto Self = StaticCastSharedPtr<FCommonActionRouteNode>(WeakSelf.Pin());
            GetRoot()->UpdateSelectedNode(Self);
        }
    }
}
void FCommonActionRouteNode::HandleWidgetDeactivated()
{
    OnWidgetDeactivated.ExecuteIfBound();
    if (!bRoutingActive) return;
    SetRoutingActive(false);
    if (Parent.IsValid() && SupportsFocusThroughParents())
    {
        auto Ancestor = Parent.Pin();
        while (Ancestor.IsValid() && !Ancestor->IsRoutingActive()) Ancestor = Ancestor->Parent.Pin();
        GetRoot()->UpdateSelectedNode(Ancestor);
    }
}
int32 FCommonActionRouteNode::GetPaintLayer() const
{
    const auto CachedWidget = Widget.IsValid() ? Widget.Get()->GetCachedWidget() : TSharedPtr<SWidget>();
    return CachedWidget.IsValid() ? CachedWidget->GetPersistentState().LayerId : INDEX_NONE;
}
TSharedPtr<FCommonActionRouteNode> FCommonActionRouteNode::SelectFocusNode(const TSharedPtr<FCommonActionRouteNode>& Candidate)
{
    if (!Candidate->Widget.IsValid() || !Candidate->Widget.Get()->IsActivated()) return nullptr;
    if (!Candidate->Widget.IsValid() || !Candidate->Widget.Get()->bSupportsActivationFocus) return nullptr;
    TSharedPtr<FCommonActionRouteNode> Best;
    auto* Child = Candidate->Children.GetData();
    auto* const End = Child + Candidate->Children.Num();
    for (; Child != End; ++Child)
    {
        auto Selected = SelectFocusNode(*Child);
        if (Selected.IsValid() && (!Best.IsValid() || Selected->GetPaintLayer() > Best->GetPaintLayer())) Best = Selected;
    }
    return Best.IsValid() ? Best : Candidate;
}
TOptional<uint8> FCommonActionRouteNode::GetInheritedOverride() const
{
    if (Widget.IsValid())
    {
        const auto Override = Widget.Get()->NativeGetActionRouteOverride();
        if (Override.IsSet()) return Override.GetValue();
    }
    const auto ParentNode = Parent.Pin();
    return ParentNode.IsValid() ? ParentNode->GetInheritedOverride() : TOptional<uint8>();
}
FCommonActionRouteInputConfig FCommonActionRouteNode::GetInheritedInputConfig() const
{
    if (Widget.IsValid())
    {
        const auto Config = Widget.Get()->NativeGetActionRouteInputConfig();
        if (Config.IsSet()) return Config.GetValue();
    }
    const auto ParentNode = Parent.Pin();
    return ParentNode.IsValid() ? ParentNode->GetInheritedInputConfig()
        : FCommonActionRouteInputConfig{ECommonInputMode::Menu, EMouseCaptureMode::NoCapture, true};
}
bool FCommonActionRouteNode::ContainsWidget(UCommonActivatableWidget* Candidate) const
{
    if (Candidate == Widget.Get()) return true;
    for (const auto& Child : Children) if (Child->ContainsWidget(Candidate)) return true;
    return false;
}
void FCommonActionRouteNode::SaveFocus()
{
    auto* Player = Owner.Get()->GetLocalPlayer();
    auto Focus = FSlateApplication::Get().GetUserFocusedWidget(LocalPlayerIndex(Player));
    if (Focus.IsValid())
    {
        auto* Ancestor = FindActivatableAncestor(Focus, Player);
        if (Focus.Get() != SavedFocus.Pin().Get() && (!Ancestor || ContainsWidget(Ancestor))) SavedFocus = Focus;
    }
}
bool FCommonActionRouteNode::IsStrictSlateDescendant(const TSharedPtr<SWidget>& Focus) const
{
    if (!Focus.IsValid() || !Widget.IsValid()) return false;
    const auto Cached = Widget.Get()->GetCachedWidget();
    for (auto Current = Focus->GetParentWidget(); Current.IsValid(); Current = Current->GetParentWidget())
        if (Current.Get() == Cached.Get()) return true;
    return false;
}
bool FCommonActionRouteNode::IsExclusiveSlateDescendant(const TSharedPtr<SWidget>& Focus) const
{
    if (!IsStrictSlateDescendant(Focus)) return false;
    for (const auto& Child : Children)
        if (Child->Widget.IsValid() && Child->Widget.Get()->bSupportsActivationFocus && Child->IsStrictSlateDescendant(Focus)) return false;
    return true;
}

FCommonActionRouteRoot::FCommonActionRouteRoot(UCommonUIActionRouter* InOwner, UCommonActivatableWidget* InWidget)
    : FCommonActionRouteNode(InOwner, InWidget) {}
TSharedRef<FCommonActionRouteRoot> FCommonActionRouteRoot::Create(UCommonUIActionRouter* InOwner, UCommonActivatableWidget* InWidget)
{
    auto Root = MakeShared<FCommonActionRouteRoot>(InOwner, InWidget);
    Root->WeakSelf = Root;
    Root->Initialize();
    return Root;
}
void FCommonActionRouteRoot::Initialize()
{
    FCommonActionRouteNode::Initialize();
    auto* Input = Owner.Get()->GetLocalPlayer()->GetSubsystem<UCommonInputSubsystem>();
    Input->NativeInputMethodChanged.AddSP(StaticCastSharedRef<FCommonActionRouteRoot>(WeakSelf.Pin().ToSharedRef()),
        &FCommonActionRouteRoot::HandleInputMethodChanged);
}
void FCommonActionRouteRoot::SetRoutingActive(bool bActive)
{
    if (bActive == bRoutingActive) return;
    FCommonActionRouteNode::SetRoutingActive(bActive);
    if (!bActive)
    {
        if (SelectedNode.IsValid())
        {
            { const auto Old = SelectedNode.Pin(); Old->SaveFocus(); }
            UpdateSelectedNode(nullptr);
        }
    }
    else if (Widget.IsValid() && Widget.Get()->IsActivated())
    {
        bool bChanged;
        {
            const auto Self = StaticCastSharedPtr<FCommonActionRouteNode>(WeakSelf.Pin());
            bChanged = UpdateSelectedNode(Self);
        }
        if (!bChanged) RefreshSelectedNode();
    }
}
bool FCommonActionRouteRoot::UpdateSelectedNode(TSharedPtr<FCommonActionRouteNode> Candidate)
{
    if (Candidate.IsValid() && !Candidate->IsRoutingActive()) return false;
    const auto Selected = Candidate.IsValid() ? SelectFocusNode(Candidate) : TSharedPtr<FCommonActionRouteNode>();
    {
        const auto Old = SelectedNode.Pin();
        if (Selected.Get() == Old.Get()) return false;
    }
    { const auto Old = SelectedNode.Pin(); if (Old.IsValid()) Old->SaveFocus(); }
    SelectedNode = Selected;
    RefreshSelectedNode();
    OnSelectedNodeChanged.ExecuteIfBound();
    return true;
}
void FCommonActionRouteRoot::RefreshSelectedNode()
{
    const auto Selected = SelectedNode.Pin();
    if (!Selected.IsValid()) return;
    auto* Router = Owner.Get();
    Router->NativeActionRouteOverrideChanged.Broadcast(Selected->GetInheritedOverride());
    if (Selected->IsRoutingActive())
    {
        const auto Config = Selected->GetInheritedInputConfig();
        Owner.Get()->NativeApplyActionRouteInputConfig(Config);
        ApplyFocus();
    }
}
void FCommonActionRouteRoot::HandleInputMethodChanged(ECommonInputType InputType)
{
    bool bCanApply = false;
    if (IsRoutingActive())
    {
        const auto Selected = SelectedNode.Pin();
        bCanApply = Selected.IsValid() && Selected->IsRoutingActive();
    }
    if (!bCanApply) return;
    if (InputType == ECommonInputType::Gamepad) ApplyFocus();
    else SelectedNode.Pin()->SaveFocus();
}
void FCommonActionRouteRoot::ApplyFocus()
{
    const auto Selected = SelectedNode.Pin();
    auto* Activatable = Selected->GetWidget();
    auto* Player = Owner.Get()->GetLocalPlayer();
    const int32 PlayerIndex = LocalPlayerIndex(Player);
    // Original resolves the owner a second time before holding pending operations.
    auto* OperationsPlayer = Owner.Get()->GetLocalPlayer();
    bool bDirectSlateFocus = false;
    auto Restore = Activatable->bSupportsActivationFocus && Activatable->bAutoRestoreFocus
        ? Selected->GetSavedFocus() : TSharedPtr<SWidget>();
    if (Restore.IsValid())
    {
        bDirectSlateFocus = FSlateApplication::Get().SetUserFocus(PlayerIndex, Restore, EFocusCause::SetDirectly);
    }
    else if (auto* Target = Activatable->NativeGetActionRouteDesiredFocusTarget())
    {
        Target->SetFocus();
    }
    else
    {
        const auto Current = FSlateApplication::Get().GetUserFocusedWidget(PlayerIndex);
        const auto Pending = OperationsPlayer->GetSlateOperations().GetUserFocusRecepient();
        if (!Selected->IsExclusiveSlateDescendant(Pending) && !Selected->IsExclusiveSlateDescendant(Current))
        {
            if (Activatable->bIsFocusable) Activatable->SetFocus();
            else if (Player->ViewportClient)
            {
                const auto Viewport = Player->ViewportClient->GetGameViewportWidget();
                if (Viewport.IsValid())
                    bDirectSlateFocus = FSlateApplication::Get().SetUserFocus(PlayerIndex, Viewport, EFocusCause::SetDirectly);
            }
        }
    }
    Restore.Reset();
    if (bDirectSlateFocus) OperationsPlayer->GetSlateOperations().CancelFocusRequest();
}
