#include "RoutePlatformCursorPolicy.h"

#include "Engine/GameViewportClient.h"
#include "Framework/Application/SlateApplication.h"
#include "HAL/IConsoleManager.h"

namespace
{
// Backing byte 0x1480d1c81, registered by the original static initializer.
bool GCommonUIAlwaysShowCursor = false;
FAutoConsoleVariableRef CVarCommonUIAlwaysShowCursor(
    TEXT("CommonUI.AlwaysShowCursor"), GCommonUIAlwaysShowCursor, TEXT(""), ECVF_Default);
}

bool RouteShouldShowCursorForTouch(UGameViewportClient* ViewportClient)
{
    bool bShowCursor = FSlateApplication::Get().IsFakingTouchEvents();
    if (ViewportClient)
    {
        // Keep this call even when Slate is already faking touch.
        bShowCursor |= ViewportClient->GetUseMouseForTouch();
    }
    return GCommonUIAlwaysShowCursor || bShowCursor;
}
