#pragma once

// Translation of the ordered portions of 144f664f0, 144f75f90, 144f5fce0
// and 144f54030. Adapter operations are required dependencies, not defaults.
// The dialog value and all callbacks remain owned by their real native types.
namespace ConfirmationWindowLifecycle
{
template <class Adapter>
void Reset(Adapter& A)
{
    using Description = typename Adapter::Description;
    A.DescriptionValue() = Description{};
    A.RemoveButtonBindings144f555e0();
    A.RefreshOptionalPanels144f7b690();
    A.RefreshContentWidgets144f7b730();
    A.SetDescriptionWidgetValue();
    A.SetDescriptionWidgetVisibilityFromIconResource();
}

template <class Adapter>
void Dismiss(Adapter& A, unsigned char Result)
{
    if (!A.IsVisible143d66ef0())
        return;
    // A copy is essential: close and callbacks can reenter the window.
    typename Adapter::Description Saved(A.DescriptionValue());
    Reset(A);
    A.Close144f55ae0();
    if (auto* Context = A.FindUIContext144ade110())
        A.Dispatch144ce33d0(*Context, Result, Saved);
}

template <class Adapter>
void Clear(Adapter& A)
{
    Dismiss(A, 3);
}

template <class Adapter>
void BotClickButton(Adapter& A)
{
    if (!A.BotControllerEligible144f54030())
        return;
    if (A.ActionCount() > 0 && A.ConfirmationButtonCount() > 0)
        A.ClickConfirmation144f5fbc0(A.FirstConfirmationButton());
    else
        Dismiss(A, 1); // 144f5fce0 ignores its button argument.
}
}
