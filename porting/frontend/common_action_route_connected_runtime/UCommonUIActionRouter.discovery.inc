// Include in the real router translation unit after the generated header.
// Also requires UCommonActivatableWidget.h and Slate/SObjectWidget.h.

namespace
{
UCommonActivatableWidget* FindActionWidgetParent(UWidget* Widget)
{
    auto SlateWidget = Widget->GetCachedWidget();
    auto* Player = Widget->GetOwningLocalPlayer();
    while (SlateWidget.IsValid())
    {
        SlateWidget = SlateWidget->GetParentWidget();
        if (SlateWidget.IsValid() && SlateWidget->GetType() == FName(TEXT("SObjectWidget")))
        {
            auto* Ancestor = Cast<UCommonActivatableWidget>(StaticCastSharedPtr<SObjectWidget>(SlateWidget)->GetWidgetObject());
            if (Ancestor) return Ancestor->GetOwningLocalPlayer() == Player ? Ancestor : nullptr;
        }
    }
    return nullptr;
}
TSharedPtr<FCommonActionRouteNode> FindActionWidgetNode(const TSharedPtr<FCommonActionRouteNode>& Node, UCommonActivatableWidget* Widget)
{
    if (Node->GetWidget() == Widget) return Node;
    for (const auto& Child : Node->GetChildren())
    {
        auto Match = FindActionWidgetNode(Child, Widget);
        if (Match.IsValid()) return Match;
    }
    return nullptr;
}
using FActionWidgetChildren = TMap<UCommonActivatableWidget*, TArray<UCommonActivatableWidget*>>;
// 1407ab300 removes the parent's map entry before creating children recursively.
void AddActionWidgetChildren(const TSharedPtr<FCommonActionRouteNode>& Node, FActionWidgetChildren& Pending)
{
    TArray<UCommonActivatableWidget*> Children;
    if (Pending.RemoveAndCopyValue(Node->GetWidget(), Children))
        for (auto* Widget : Children) AddActionWidgetChildren(Node->AddChild(Widget), Pending);
}
}

UCommonUIActionRouter::UCommonUIActionRouter(const FObjectInitializer& ObjectInitializer)
    : Super(), NativeStandaloneActionRoute(FCommonActionBindingRoute::Create(this))
{
    (void)ObjectInitializer; // Target ULocalPlayerSubsystem exposes only its default constructor.
    // 140797b40 also binds global input/debug callbacks outside this component.
    // Their constructor integration remains with the owner, not fake callbacks.
}
TSharedPtr<FCommonActionRouteNode> UCommonUIActionRouter::NativeFindActionRoute(UCommonActivatableWidget* Widget) const
{
    if (!Widget) return nullptr;
    for (const auto& Root : NativeActionRouteRoots)
    {
        if (Widget->bIsModal)
        {
            if (Root->GetWidget() == Widget) return Root;
        }
        else
        {
            auto Match = FindActionWidgetNode(Root, Widget);
            if (Match.IsValid()) return Match;
        }
    }
    return nullptr;
}
TSharedPtr<FCommonActionRouteNode> UCommonUIActionRouter::NativeAcquireActionRoute(UWidget* Widget) const
{
    auto* Activatable = Cast<UCommonActivatableWidget>(Widget);
    auto Route = NativeFindActionRoute(Activatable);
    if (!Route.IsValid() && (!Activatable || !Activatable->bIsModal))
        if (auto* Parent = FindActionWidgetParent(Widget)) Route = NativeFindActionRoute(Parent);
    return Route;
}
FCommonActionRouteOwnerRecord& UCommonUIActionRouter::NativeFindOrAddActionOwner(UWidget* Widget)
{
    for (auto& Record : NativePendingActionOwners) if (Record.Widget.Get() == Widget) return Record;
    return NativePendingActionOwners[NativePendingActionOwners.Emplace(Widget)];
}
int32 UCommonUIActionRouter::NativeRegisterActionBinding(UWidget* BindingOwner, const FCommonActionBindingDescriptor& Descriptor)
{
    if (bNativeActionRoutingBlocked) return INDEX_NONE;
    const int32 Id = CommonActionBindingData::Create(Descriptor, BindingOwner);
    if (Id < 0 || !CommonActionBindingData::Find(Id)) return INDEX_NONE;
    auto* Activatable = Cast<UCommonActivatableWidget>(BindingOwner);
    const auto Route = Activatable ? NativeFindActionRoute(Activatable) : NativeAcquireActionRoute(BindingOwner);
    if (Route.IsValid()) Route->Associate(*CommonActionBindingData::Find(Id));
    else
    {
        // Original releases the shared temporary before testing the raw identity.
        SWidget* Cached;
        { const auto SlateWidget = BindingOwner->GetCachedWidget(); Cached = SlateWidget.Get(); }
        if (Cached) NativeFindOrAddActionOwner(BindingOwner).BindingHandles.AddUnique(Id);
    }
    return Id;
}
void UCommonUIActionRouter::NativeRemoveActionBindingFromRoute(int32 Handle)
{
    auto* Binding = CommonActionBindingData::Find(Handle);
    if (!Binding) return;
    if (Binding->Route.IsValid())
    {
        const auto Route = Binding->Route.Pin();
        Route->Disassociate(Handle);
    }
    else
    {
        auto* BindingOwner = Binding->Owner.Get();
        for (auto& Record : NativePendingActionOwners)
            if (Record.Widget.Get() == BindingOwner) { Record.BindingHandles.Remove(Handle); break; }
    }
}
void UCommonUIActionRouter::NativeRemoveWidgetActionBindings(UCommonUserWidget* Widget)
{
    const int32 Pending = NativePendingActionOwners.IndexOfByPredicate([Widget](const FCommonActionRouteOwnerRecord& Record)
    { return Record.Widget.Get() == Widget; });
    if (Pending != INDEX_NONE)
    {
        NativePendingActionOwners.RemoveAt(Pending, 1, true);
        return;
    }
    NativeStandaloneActionRoute->Disassociate(Widget->NativeGetRegisteredActionBindingHandles());
    const auto Route = NativeAcquireActionRoute(Widget);
    if (Route.IsValid()) Route->Disassociate(Widget->NativeGetRegisteredActionBindingHandles());
}
void UCommonUIActionRouter::NativeReassociateWidgetActionBindings(UCommonUserWidget* Widget)
{
    const auto Route = NativeAcquireActionRoute(Widget);
    if (Route.IsValid())
    {
        for (int32 Handle : Widget->NativeGetRegisteredActionBindingHandles())
            if (auto* Binding = CommonActionBindingData::Find(Handle))
            {
                if (Binding->Flag20) NativeStandaloneActionRoute->Associate(*Binding);
                else Route->Associate(*Binding);
            }
    }
    else if (Widget->NativeGetRegisteredActionBindingHandles().Num() > 0)
    {
        // Rebuild appends the full surviving list, unlike registration's AddUnique.
        NativeFindOrAddActionOwner(Widget).BindingHandles.Append(Widget->NativeGetRegisteredActionBindingHandles());
    }
}
bool UCommonUIActionRouter::NativeRegisterInputPreProcessor(UWidget* BindingOwner, const TSharedPtr<IInputProcessor>& Processor, int32 Index)
{
    const auto Route = NativeAcquireActionRoute(BindingOwner);
    if (Route.IsValid()) Route->AddInputPreProcessor(Processor, Index);
    else
    {
        SWidget* Cached;
        { const auto SlateWidget = BindingOwner->GetCachedWidget(); Cached = SlateWidget.Get(); }
        if (!Cached) return false;
        auto& Entries = NativeFindOrAddActionOwner(BindingOwner).InputProcessors;
        for (auto& Entry : Entries)
            if (Entry.InputProcessor.IsValid() && Entry.InputProcessor.Get() == Processor.Get())
            { Entry.Index = Index; return true; }
        Entries.Add({Processor, Index});
    }
    return true;
}
void UCommonUIActionRouter::NativeRegisterRoutedWidget(UWidget* Widget)
{
    const auto Route = NativeAcquireActionRoute(Widget);
    if (Route.IsValid()) Route->AddRoutedWidget(Widget);
    else NativeFindOrAddActionOwner(Widget).bRouteWidget = true;
}
void UCommonUIActionRouter::NativeHandleActivatableWidgetRebuilt(UCommonActivatableWidget* Widget)
{
    if (Widget->GetOwningLocalPlayer() == GetLocalPlayer()) NativePendingActionWidgets.Add(Widget);
}
void UCommonUIActionRouter::NativeInitializeActionRouteDiscovery(FSubsystemCollectionBase& Collection)
{
    Collection.InitializeDependency(UCommonInputSubsystem::StaticClass());
    GetCommonActivatableWidgetRebuilt().AddUObject(this, &UCommonUIActionRouter::NativeHandleActivatableWidgetRebuilt);
}
void UCommonUIActionRouter::NativeTickActionRouteDiscovery()
{
    if (NativePendingActionOwners.Num() > 0 || NativePendingActionWidgets.Num() > 0) NativeProcessRebuiltActionWidgets();
    if (!bNativeActionRoutingEnabled) return;
    int32 BestLayer = NativeActiveRouteRoot.IsValid() ? NativeActiveRouteRoot->GetPaintLayer() : INDEX_NONE;
    auto Selected = NativeActiveRouteRoot;
    auto* Root = NativeActionRouteRoots.GetData();
    auto* const End = Root + NativeActionRouteRoots.Num();
    for (; Root != End; ++Root)
    {
        auto* Widget = (*Root)->GetWidget();
        if (Widget && Widget->IsActivated())
        {
            const int32 Layer = (*Root)->GetPaintLayer();
            if (Layer > BestLayer) { BestLayer = Layer; Selected = *Root; }
        }
    }
    if (Selected.Get() != NativeActiveRouteRoot.Get()) NativeSetActiveActionRouteRoot(Selected);
}
void UCommonUIActionRouter::NativeHandleActionRootActivated(TWeakPtr<FCommonActionRouteRoot> WeakRoot)
{
    const auto Root = WeakRoot.Pin();
    if (NativeActionRouteRoots.Contains(Root) && Root->GetPaintLayer() > 0)
    {
        const int32 OldLayer = NativeActiveRouteRoot.IsValid() ? NativeActiveRouteRoot->GetPaintLayer() : INDEX_NONE;
        if (Root->GetPaintLayer() > OldLayer) NativeSetActiveActionRouteRoot(Root);
    }
}
void UCommonUIActionRouter::NativeHandleActionRootDeactivated(TWeakPtr<FCommonActionRouteRoot> WeakRoot)
{
    if (NativeActiveRouteRoot.IsValid() && NativeActiveRouteRoot.Get() == WeakRoot.Pin().Get()) NativeSetActiveActionRouteRoot(nullptr);
}
void UCommonUIActionRouter::NativeHandleActionRootReleased(UCommonActivatableWidget* Widget)
{
    Widget->NativeWidgetReleased.RemoveAll(this);
    const int32 Index = NativeActionRouteRoots.IndexOfByPredicate([Widget](const TSharedPtr<FCommonActionRouteRoot>& Root)
    { return Root->GetWidget() == Widget; });
    if (Index == INDEX_NONE) return;
    if (NativeActiveRouteRoot.IsValid() && NativeActiveRouteRoot == NativeActionRouteRoots[Index])
        NativeHandleActionRootDeactivated(NativeActiveRouteRoot);
    const TWeakPtr<FCommonActionRouteRoot> RetainController = NativeActionRouteRoots[Index];
    NativeActionRouteRoots.RemoveAt(Index, 1, true);
    if (NativeActionRouteRoots.Num() == 0) NativeActionInputConfig.Reset();
}
void UCommonUIActionRouter::NativeProcessRebuiltActionWidgets()
{
    FActionWidgetChildren PendingChildren;
    TArray<UCommonActivatableWidget*> NewRoots;
    auto* Widget = NativePendingActionWidgets.GetData();
    auto* const WidgetEnd = Widget + NativePendingActionWidgets.Num();
    for (; Widget != WidgetEnd; ++Widget)
    {
        if (!Widget->IsValid() || !Widget->Get()->GetCachedWidget().IsValid()) continue;
        auto* Activatable = Widget->Get();
        if (!Activatable->bIsModal)
        {
            if (auto* Parent = FindActionWidgetParent(Activatable))
            { PendingChildren.FindOrAdd(Parent).Add(Activatable); continue; }
        }
        NewRoots.Add(Activatable);
    }
    for (auto* RootWidget : NewRoots)
    {
        const auto Root = FCommonActionRouteRoot::Create(this, RootWidget);
        Root->OnWidgetActivated.BindUObject(this, &UCommonUIActionRouter::NativeHandleActionRootActivated, TWeakPtr<FCommonActionRouteRoot>(Root));
        Root->OnWidgetDeactivated.BindUObject(this, &UCommonUIActionRouter::NativeHandleActionRootDeactivated, TWeakPtr<FCommonActionRouteRoot>(Root));
        RootWidget->NativeWidgetReleased.AddUObject(this, &UCommonUIActionRouter::NativeHandleActionRootReleased, RootWidget);
        NativeActionRouteRoots.Add(Root);
        AddActionWidgetChildren(Root, PendingChildren);
        if (RootWidget->IsActivated()) NativeHandleActionRootActivated(Root);
    }
    int32 PreviousRemaining = INDEX_NONE;
    while (PendingChildren.Num() > 0 && PendingChildren.Num() != PreviousRemaining)
    {
        PreviousRemaining = PendingChildren.Num();
        // Keep sparse-map order while recursive creation removes descendant keys.
        TArray<UCommonActivatableWidget*> Parents;
        PendingChildren.GenerateKeyArray(Parents);
        for (auto* Parent : Parents)
            if (PendingChildren.Contains(Parent))
                if (const auto Route = NativeFindActionRoute(Parent)) AddActionWidgetChildren(Route, PendingChildren);
    }
    auto* Record = NativePendingActionOwners.GetData();
    auto* const RecordEnd = Record + NativePendingActionOwners.Num();
    for (; Record != RecordEnd; ++Record)
    {
        auto* BindingOwner = Record->Widget.Get();
        if (!BindingOwner || !BindingOwner->GetCachedWidget().IsValid()) continue;
        const auto Route = NativeAcquireActionRoute(BindingOwner);
        for (int32 Handle : Record->BindingHandles)
            if (auto* Binding = CommonActionBindingData::Find(Handle))
            {
                if (Binding->Flag20) NativeStandaloneActionRoute->Associate(*Binding);
                else if (Route.IsValid()) Route->Associate(*Binding);
            }
        if (Route.IsValid())
        {
            if (Record->bRouteWidget) Route->AddRoutedWidget(BindingOwner);
            for (const auto& Entry : Record->InputProcessors) Route->AddInputPreProcessor(Entry.InputProcessor, Entry.Index);
        }
    }
    NativePendingActionWidgets.Reset();
    NativePendingActionOwners.Reset();
}
