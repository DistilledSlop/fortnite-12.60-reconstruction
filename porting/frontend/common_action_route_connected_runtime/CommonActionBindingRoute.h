#pragma once

#include "CoreMinimal.h"
#include "CommonActionBindingData.h"
#include "CommonActionRouteTypes.h"
#include "ECommonInputType.h"

class UCommonUIActionRouter;
class UCommonActivatableWidget;
class UCommonUserWidget;
class UWidget;
class ULocalPlayer;
class IInputProcessor;
class SWidget;
class UGameViewportClient;
struct FFocusEvent;
class FWeakWidgetPath;
class FWidgetPath;
class FCommonActionRouteNode;
class FCommonActionRouteRoot;

bool RouteShouldShowCursorForTouch(UGameViewportClient* ViewportClient); // 1407fb160

// Descriptive identifiers for original vtables 1459ecdd0/df8/ece30. Ordinary
// source members represent the original state; source layout is not an ABI shim.
class FCommonActionBindingRoute
{
public:
    explicit FCommonActionBindingRoute(UCommonUIActionRouter* InOwner);
    static TSharedRef<FCommonActionBindingRoute> Create(UCommonUIActionRouter* Owner);
    virtual ~FCommonActionBindingRoute() = default;
    virtual bool IsRoutingActive() const; // original virtual +18
    virtual ECommonActionHoldDispatchResult ProcessHoldInput(uint8 InputMode, FKey Key, EInputEvent Event); // virtual +08
    virtual bool ProcessNormalInput(uint8 InputMode, FKey Key, EInputEvent Event); // virtual +10
    virtual bool CanReceiveInput(UObject* BindingOwner) const; // virtual +20
    void Associate(FCommonActionBindingRecord& Record); // 1407a90c0
    void Disassociate(int32 Id); // 1407f0ab0
    void Disassociate(const TArray<int32>& Handles); // 1407f0c40
    static void Unregister(int32& Handle); // 1408018c0
    const TArray<int32>& GetBindingHandles() const { return BindingHandles; }
protected:
    void NotifyBindingsIfActive();
    TWeakPtr<FCommonActionBindingRoute> WeakSelf; // original shared-from-this +08/+10
    TArray<int32> BindingHandles; // original +18
    TWeakObjectPtr<UCommonUIActionRouter> Owner; // original +28
    int32 PendingBinding = INDEX_NONE; // original +30, no new producer claimed
};

struct FCommonActionRouteProcessor
{
    int32 Index;
    TSharedPtr<IInputProcessor> InputProcessor;
};

class FCommonActionRouteNode : public FCommonActionBindingRoute
{
public:
    FCommonActionRouteNode(UCommonUIActionRouter* InOwner, UCommonActivatableWidget* InWidget);
    virtual ~FCommonActionRouteNode();
    virtual bool IsRoutingActive() const override;
    virtual ECommonActionHoldDispatchResult ProcessHoldInput(uint8 InputMode, FKey Key, EInputEvent Event) override;
    virtual bool ProcessNormalInput(uint8 InputMode, FKey Key, EInputEvent Event) override;
    virtual bool CanReceiveInput(UObject* BindingOwner) const override;
    virtual void Initialize(); // original virtual +28, after shared-self installation
    virtual void SetRoutingActive(bool bActive); // original virtual +30
    TSharedRef<FCommonActionRouteNode> AddChild(UCommonActivatableWidget* ChildWidget);
    void RemoveChildForWidget(UCommonActivatableWidget* ChildWidget);
    void AddInputPreProcessor(const TSharedPtr<IInputProcessor>& Processor, int32 Index);
    void AddRoutedWidget(UWidget* InWidget); // 1407aa190, original +a8 payload
    void CollectRoutedWidgets(TArray<UWidget*>& OutWidgets); // 1407aa7a0
    void HandleWidgetActivated(); // 1407cca90
    void HandleWidgetDeactivated(); // 1407ccc50
    bool SupportsFocusThroughParents() const; // 1407b94e0
    TSharedPtr<FCommonActionRouteRoot> GetRoot(); // 1407c46e0
    static TSharedPtr<FCommonActionRouteNode> SelectFocusNode(const TSharedPtr<FCommonActionRouteNode>& Candidate);
    TOptional<uint8> GetInheritedOverride() const; // 1407bd840, inner optional only
    FCommonActionRouteInputConfig GetInheritedInputConfig() const; // 1407bd980
    void SaveFocus(); // 1407ad210
    bool ContainsWidget(UCommonActivatableWidget* Candidate) const; // 1407d00e0
    bool IsStrictSlateDescendant(const TSharedPtr<SWidget>& Focus) const; // 1407cfcd0
    bool IsExclusiveSlateDescendant(const TSharedPtr<SWidget>& Focus) const; // 1407cf170
    int32 GetPaintLayer() const; // 1407c3c30
    TSharedPtr<SWidget> GetSavedFocus() const { return SavedFocus.Pin(); }
    UCommonActivatableWidget* GetWidget() const;
    const TArray<TSharedPtr<FCommonActionRouteNode>>& GetChildren() const { return Children; }
    FSimpleDelegate OnWidgetActivated; // original callable +38
    FSimpleDelegate OnWidgetDeactivated; // original callable +48
protected:
    TWeakObjectPtr<UCommonActivatableWidget> Widget; // +58
    TWeakPtr<FCommonActionRouteNode> Parent; // +60
    TArray<TSharedPtr<FCommonActionRouteNode>> Children; // +70
    TWeakPtr<SWidget> SavedFocus; // +80
    bool bRoutingActive = false; // +90
    TArray<FCommonActionRouteProcessor> InputProcessors; // +98
    TArray<TWeakObjectPtr<UWidget>> RoutedWidgets; // +a8
};

class FCommonActionRouteRoot : public FCommonActionRouteNode
{
public:
    FCommonActionRouteRoot(UCommonUIActionRouter* InOwner, UCommonActivatableWidget* InWidget);
    static TSharedRef<FCommonActionRouteRoot> Create(UCommonUIActionRouter* Owner, UCommonActivatableWidget* Widget);
    virtual void Initialize() override;
    virtual void SetRoutingActive(bool bActive) override;
    bool UpdateSelectedNode(TSharedPtr<FCommonActionRouteNode> Candidate); // 1408030f0
    void RefreshSelectedNode(); // 1407aa8d0
    void ApplyFocus(); // 1407bef20
    void HandleInputMethodChanged(ECommonInputType InputType);
    TSharedPtr<FCommonActionRouteNode> GetSelectedNode() const { return SelectedNode.Pin(); }
    FSimpleDelegate OnSelectedNodeChanged; // +b8
private:
    TWeakPtr<FCommonActionRouteNode> SelectedNode; // +c8
};
