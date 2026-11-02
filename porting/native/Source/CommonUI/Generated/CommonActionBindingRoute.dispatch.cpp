#include "CommonActionBindingRoute.h"
#include "Components/Widget.h"
#include "UCommonActivatableWidget.h"
#include "HAL/PlatformTime.h"
#include "Widgets/SWidget.h"
#include "CommonButtonMetaData.h"
#include "UCommonButton.h"

// 1407adc50. Ordinary Slate interfaces preserve the original attribute getters,
// path validation virtual and first-matching metadata traversal.
bool NativeCommonActionCanReceiveInput(const SWidget& Widget)
{
    if (!Widget.IsEnabled()) return false;
    const auto Visibility = Widget.GetVisibility();
    if (!Visibility.IsVisible() || !Visibility.AreChildrenHitTestVisible()) return false;
    const auto Parent = Widget.GetParentWidget();
    if (Parent.IsValid() && !Parent->ValidatePathToChild(const_cast<SWidget*>(&Widget))) return false;
    const auto ButtonMetaData = Widget.GetMetaData<FCommonButtonMetaData>();
    return !ButtonMetaData.IsValid() || ButtonMetaData->Button.Get()->NativeIsInteractionEnabled();
}

namespace
{
void ResetHold(FCommonActionBindingRecord& Record)
{
    Record.State = -1.0;
    Record.ProgressListeners.Broadcast(0.0f); // 1407ac590
}

// 140802a10. Seconds uses the same platform clock and 2^24 offset as the
// original QueryPerformanceCounter conversion; State is shared binding data.
bool UpdateHold(FCommonActionBindingRecord& Record, float HoldTime)
{
    const float Elapsed = Record.State < 0.0 ? 0.0f
        : static_cast<float>(FPlatformTime::Seconds() - Record.State);
    const float Progress = FMath::Clamp(Elapsed / HoldTime, 0.0f, 1.0f);
    Record.ProgressListeners.Broadcast(Progress);
    if (Progress >= 1.0f)
    {
        ResetHold(Record);
        Record.Click.ExecuteIfBound();
        return true;
    }
    return false;
}

TSharedPtr<SWidget> CachedSlate(UObject* Object)
{
    // Record owners are UWidgets: the source calls UWidget::GetCachedWidget
    // directly, after the owner weak-object resolution.
    return Object ? static_cast<UWidget*>(Object)->GetCachedWidget() : nullptr;
}
}

bool FCommonActionBindingRoute::CanReceiveInput(UObject* BindingOwner) const
{
    auto Slate = CachedSlate(BindingOwner);
    if (!Slate.IsValid()) return false;
    do
    {
        if (!NativeCommonActionCanReceiveInput(*Slate)) return false;
        Slate = Slate->GetParentWidget();
    } while (Slate.IsValid());
    return true;
}

bool FCommonActionRouteNode::CanReceiveInput(UObject* BindingOwner) const
{
    auto Slate = CachedSlate(BindingOwner);
    if (!Slate.IsValid()) return false;
    const auto Boundary = Widget.Get()->GetCachedWidget();
    if (!Boundary.IsValid()) return false;
    while (Slate.IsValid() && Slate != Boundary)
    {
        if (!NativeCommonActionCanReceiveInput(*Slate)) return false;
        Slate = Slate->GetParentWidget();
    }
    // 1407d09ce accepts reaching the boundary OR exhausting the parent chain.
    return true;
}

ECommonActionHoldDispatchResult FCommonActionBindingRoute::ProcessHoldInput(
    uint8 InputMode, FKey Key, EInputEvent Event)
{
    // 1407e35b0. Binding priority is association order; each record's hold
    // mappings are visited in order. Non-consuming mappings continue scanning.
    for (int32 Handle : BindingHandles)
    {
        auto* Record = CommonActionBindingData::Find(Handle);
        if (!Record || (InputMode != static_cast<uint8>(ECommonInputMode::All)
            && InputMode != Record->InputMode)) continue;
        for (const auto& Mapping : Record->Hold)
        {
            if (Mapping.Key != Key) continue;
            const bool bPersistent = Record->Flag20 && Record->Flag22;
            if (!bPersistent && !CanReceiveInput(Record->Owner.Get())) continue;
            auto Result = ECommonActionHoldDispatchResult::Unhandled;
            if (Event == IE_Pressed)
            {
                if (auto* Previous = CommonActionBindingData::Find(PendingBinding)) ResetHold(*Previous);
                PendingBinding = Handle;
                Record->State = FPlatformTime::Seconds();
                Record->ProgressListeners.Broadcast(0.0f);
                Result = ECommonActionHoldDispatchResult::Handled;
            }
            else if (!(Record->State < 0.0))
            {
                if (Event == IE_Repeat)
                {
                    if (UpdateHold(*Record, Mapping.Time))
                    {
                        PendingBinding = INDEX_NONE;
                        Result = ECommonActionHoldDispatchResult::Handled;
                    }
                }
                else if (Event == IE_Released)
                {
                    const float Elapsed = static_cast<float>(FPlatformTime::Seconds() - Record->State);
                    PendingBinding = INDEX_NONE;
                    ResetHold(*Record);
                    if (Elapsed <= 0.25f && Elapsed < Mapping.Time)
                        Result = ECommonActionHoldDispatchResult::GeneratePress;
                }
            }
            if (Record->Flag21) return Result;
        }
    }
    if (auto* Previous = CommonActionBindingData::Find(PendingBinding)) ResetHold(*Previous);
    PendingBinding = INDEX_NONE;
    return ECommonActionHoldDispatchResult::Unhandled;
}

ECommonActionHoldDispatchResult FCommonActionRouteNode::ProcessHoldInput(
    uint8 InputMode, FKey Key, EInputEvent Event)
{
    // 1407e39c0: children have priority over the current node.
    if (!IsRoutingActive()) return ECommonActionHoldDispatchResult::Unhandled;
    for (const auto& Child : Children)
    {
        const auto Result = Child->ProcessHoldInput(InputMode, Key, Event);
        if (Result != ECommonActionHoldDispatchResult::Unhandled) return Result;
    }
    return FCommonActionBindingRoute::ProcessHoldInput(InputMode, Key, Event);
}

bool FCommonActionBindingRoute::ProcessNormalInput(uint8 InputMode, FKey Key, EInputEvent Event)
{
    // 1407e4290: only the per-binding configured event invokes the click.
    for (int32 Handle : BindingHandles)
    {
        auto* Record = CommonActionBindingData::Find(Handle);
        if (!Record || (InputMode != static_cast<uint8>(ECommonInputMode::All)
            && InputMode != Record->InputMode)) continue;
        for (const auto& Mapping : Record->Press)
        {
            if (Mapping.Key != Key || Record->Event != Event) continue;
            if (!(Record->Flag20 && Record->Flag22) && !CanReceiveInput(Record->Owner.Get())) continue;
            Record->State = -1.0;
            Record->Click.ExecuteIfBound();
            if (Record->Flag21) return true;
        }
    }
    return false;
}

bool FCommonActionRouteNode::ProcessNormalInput(uint8 InputMode, FKey Key, EInputEvent Event)
{
    // 1407e4440: same children-first priority as the hold virtual.
    if (!IsRoutingActive()) return false;
    for (const auto& Child : Children)
        if (Child->ProcessNormalInput(InputMode, Key, Event)) return true;
    return FCommonActionBindingRoute::ProcessNormalInput(InputMode, Key, Event);
}
