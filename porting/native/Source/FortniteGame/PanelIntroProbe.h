#pragma once
#include "CoreMinimal.h"
#include "UCommonActivatablePanel.h"
#include "PanelIntroProbe.generated.h"

UCLASS(Transient, NotBlueprintable)
class UPanelIntroProbeWidget : public UCommonActivatablePanel
{
    GENERATED_BODY()
public:
    FString ProbeCase;
    bool DeferIntro = false;
    bool ReenterCompletion = false;
    virtual void ProcessEvent(UFunction* Function, void* Parameters) override;
protected:
    virtual void NativeIntroComplete() override;
    virtual void NativeInputMethodChangedHook(ECommonInputType Type) override;
    virtual uint32 NativeGetGlobalInputPriorityFilter() const override { return 0x12345678; }
};
UCLASS(Transient, NotBlueprintable)
class UPanelIntroProbeSink : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() UPanelIntroProbeWidget* Widget = nullptr;
    void Pre(UCommonActivatablePanel* Panel);
    void Post(UCommonActivatablePanel* Panel);
    void Native();
    UFUNCTION() void BaseDynamic();
    UFUNCTION() void PanelDynamic(UCommonActivatablePanel* Panel);
};
void RegisterPanelIntroProbeDriver();
