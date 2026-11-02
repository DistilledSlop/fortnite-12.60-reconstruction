// Source candidate; unregistered and unbuilt. Descriptive helper identifier.
// Original 1407be930 consumes the incoming shared pointer and starts at its parent.
#pragma once

#include "CoreMinimal.h"
#include "Slate/SObjectWidget.h"
#include "UCommonActivatableWidget.h"

namespace CommonActionFocusRecovery
{
inline UCommonActivatableWidget* FindActivatableAncestor(
    TSharedPtr<SWidget> Widget, const ULocalPlayer* OwningLocalPlayer)
{
    while (Widget.IsValid())
    {
        Widget = Widget->GetParentWidget();
        if (Widget.IsValid() && Widget->GetType() == FName(TEXT("SObjectWidget")))
        {
            UCommonActivatableWidget* Activatable = Cast<UCommonActivatableWidget>(
                StaticCastSharedPtr<SObjectWidget>(Widget)->GetWidgetObject());
            if (Activatable)
            {
                // A different player's first activatable ancestor terminates the search.
                return Activatable->GetOwningLocalPlayer() == OwningLocalPlayer
                    ? Activatable : nullptr;
            }
        }
    }
    return nullptr;
}
}
