#pragma once
#include "UCommonButton.h"
#include "UCommonButtonStyle.h"
#include "CommonButtonProductionFlowProbe.generated.h"

UCLASS() class UButtonFlowStyleA : public UCommonButtonStyle { GENERATED_BODY() };
UCLASS() class UButtonFlowStyleB : public UCommonButtonStyle { GENERATED_BODY() };

UCLASS()
class UButtonFlowWidget : public UCommonButton
{
    GENERATED_BODY()
public:
    int32 Initialized = 0, Clicks = 0, Doubles = 0, TextEvents = 0;
    bool bWeakAbsentAtInitialized = false, bReenterStyle = false;
    UCommonButtonInternal* Internal() const { return NativeInternalButton.Get(); }
    bool Selected() const { return bNativeSelected; }
    void Select(bool Value) { NativeSetIsSelected(Value, false); }
    void Configure()
    {
        ClickMethod = EButtonClickMethod::MouseDown;
        TouchMethod = EButtonTouchMethod::PreciseTap;
        PressMethod = EButtonPressMethod::ButtonPress;
        bIsFocusable = true;
        bNativeButtonEnabled = true; bNativeInteractionEnabled = true;
    }
protected:
    virtual void NativeOnInitialized() override;
    virtual void NativeOnClicked() override;
    virtual void NativeOnDoubleClicked() override;
    virtual void NativeOnCurrentTextStyleChanged() override;
};
void RegisterCommonButtonProductionFlowProbeDriver();
