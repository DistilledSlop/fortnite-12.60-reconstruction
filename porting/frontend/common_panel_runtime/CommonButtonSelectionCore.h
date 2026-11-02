#pragma once

// Original CommonButton public/internal selection and base notification order.
// The UObject adapter must supply real state, lifecycle and callback operations.
// This core does not create selection state or replace unresolved operations.
namespace CommonButtonSelectionCore
{
enum class ButtonStyle { Normal, Selected, Disabled };

template<class Operations>
void RefreshStyle(Operations& Ops)
{
    auto* Internal = Ops.ResolveInternalButton();
    if (!Internal) return;
    const ButtonStyle Style = Ops.IsSelected() ? ButtonStyle::Selected
        : Ops.IsButtonEnabledFlag() ? ButtonStyle::Normal : ButtonStyle::Disabled;
    Ops.SetInternalStyle(Internal, Style);
    Ops.TextStyleChanged();
}

template<class Operations>
void SetInternal(bool Selected, bool GiveFeedback, bool Notify, Operations& Ops)
{
    Ops.SetSelected(Selected);
    Ops.RefreshStyle();
    // Style notifications may synchronously change selection.
    if (Ops.IsSelected())
    {
        Ops.OnSelected(Notify);
        if (!Ops.IsToggleable())
        {
            auto* Internal = Ops.ResolveInternalButton();
            const bool Enabled = Ops.IsInteractableWhenSelected();
            Ops.SetInternalInteraction(Internal, Enabled);
        }
        if (GiveFeedback) Ops.PlaySelectionFeedback();
    }
    else
    {
        auto* Internal = Ops.ResolveInternalButton();
        const bool Enabled = Ops.IsInteractionEnabledFlag();
        Ops.SetInternalInteraction(Internal, Enabled);
        Ops.OnDeselected(Notify);
    }
    Ops.RefreshInputAction();
}

template<class Operations>
void Set(bool Selected, bool GiveFeedback, Operations& Ops)
{
    const bool WasHovered = Ops.IsHovered();
    if (Ops.IsSelectable() && Selected != Ops.IsSelected())
    {
        if (Selected) SetInternal(true, GiveFeedback, true, Ops);
        else if (Ops.IsToggleable()) SetInternal(false, true, true, Ops);
    }
    const bool Hovered = Ops.IsHovered();
    if (Hovered != WasHovered)
    {
        if (Hovered) Ops.OnHovered();
        else Ops.OnUnhovered();
    }
}

template<class Operations>
void Clear(Operations& Ops)
{
    SetInternal(false, false, true, Ops);
}

// Both base callbacks always emit their Blueprint event and text-style event.
// Notify controls only the native singlecast and reflected multicast.
template<class Operations>
void NotifySelection(bool Selected, bool Notify, Operations& Ops)
{
    if (Selected) Ops.BlueprintSelected();
    else Ops.BlueprintDeselected();
    if (Notify)
    {
        Ops.SinglecastSelected(Selected);
        Ops.MulticastSelected(Selected);
    }
    Ops.TextStyleChanged();
}
}
