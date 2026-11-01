#pragma once
#include "CoreMinimal.h"
#include "UCommonActivatablePanel.h"
#include "UCommonActionHandlerInterface.h"
#include "PanelOutroProbe.generated.h"

UCLASS(Transient, NotBlueprintable)
class UPanelOutroProbeWidget : public UCommonActivatablePanel, public ICommonActionHandlerInterface
{
    GENERATED_BODY()
public:
    FString ProbeCase;
    bool DeferOutro = false;
    virtual void ProcessEvent(UFunction* Function, void* Parameters) override;
protected:
    virtual void NativeOutroComplete() override;
};
UCLASS(Transient, NotBlueprintable)
class UPanelOutroProbeSink : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() UPanelOutroProbeWidget* Widget = nullptr;
    UPROPERTY() UCommonInputManager* SwapManager = nullptr;
    UPROPERTY() UCommonInputManager* PreviousManager = nullptr;
    int32 ProgressCount = 0;
    void Pre(UCommonActivatablePanel* Panel);
    void Post(UCommonActivatablePanel* Panel);
    void Native();
    void Progress(float Value);
    UFUNCTION() void BaseDynamic();
    UFUNCTION() void PanelDynamic(UCommonActivatablePanel* Panel);
};
void RegisterPanelOutroProbeDriver();
