#pragma once
#include "CoreMinimal.h"
#include "UCommonButtonInternal.h"
#include "ButtonInternalProbe.generated.h"

UCLASS(Transient, NotBlueprintable)
class UButtonInternalProbeWidget : public UCommonButtonInternal
{
    GENERATED_BODY()
public:
    TSharedPtr<SButton> BaseButton() const { return MyButton; }
    TSharedPtr<SWidget> Box() const;
    TSharedPtr<SButton> CommonButton() const;
    void ReleaseProbe() { ReleaseSlateResources(true); }
    FReply Click() { return SlateHandleCommonClicked(); }
    FReply DoubleClick() { return SlateHandleCommonDoubleClicked(); }
    void Press() { SlateHandleCommonPressed(); }
    void Release() { SlateHandleCommonReleased(); }
    void Focus() { SlateHandleCommonFocusReceived(); }
};

UCLASS(Transient, NotBlueprintable)
class UButtonInternalProbeSink : public UObject
{
    GENERATED_BODY()
public:
    FReply NativeDouble();
    void NativeFocus();
    UFUNCTION() void DynamicDouble();
    UFUNCTION() void DynamicClick();
    UFUNCTION() void DynamicPress();
    UFUNCTION() void DynamicRelease();
};
void RegisterButtonInternalProbeDriver();
