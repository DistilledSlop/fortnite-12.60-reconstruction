#pragma once
#include "UCommonActionWidget.h"
#include "UCommonInputControllerData.h"
#include "CommonActionWidgetFlowProbe.generated.h"

UCLASS()
class UActionWidgetFlowController : public UCommonInputControllerData
{
    GENERATED_BODY()
};

UCLASS()
class UActionWidgetFlowWidget : public UCommonActionWidget
{
    GENERATED_BODY()
public:
    bool bObservedInputMethod = false;
    bool bObservedGamepad = false;
    ESlateVisibility ObservedVisibility = ESlateVisibility::Visible;
    UFUNCTION()
    void ObserveInputMethodChanged(bool bGamepad)
    {
        bObservedInputMethod = true;
        bObservedGamepad = bGamepad;
        ObservedVisibility = GetVisibility();
    }
    const FSlateBrush& CachedBrush() const { return NativeCachedBrush; }
    TSharedPtr<SBox> Box() const { return NativeBox; }
    TSharedPtr<SImage> Icon() const { return NativeIcon; }
    TSharedPtr<SImage> Progress() const { return NativeProgress; }
    TSharedPtr<SImage> Rim() const { return NativeRim; }
};

void RegisterCommonActionWidgetFlowProbeDriver();
