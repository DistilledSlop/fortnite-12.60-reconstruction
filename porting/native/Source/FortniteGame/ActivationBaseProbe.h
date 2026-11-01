#pragma once
#include "CoreMinimal.h"
#include "UCommonActivatableWidget.h"
#include "ActivationBaseProbe.generated.h"

// Diagnostic only; no original cooked Blueprint behavior is implemented here.
UCLASS(Transient, NotBlueprintable)
class UActivationBaseProbeWidget : public UCommonActivatableWidget
{
    GENERATED_BODY()
public:
    FString ProbeCase;
    bool ReenterOnActivation = false;
    virtual void ProcessEvent(UFunction* Function, void* Parameters) override;
};

UCLASS(Transient, NotBlueprintable)
class UActivationBaseProbeSink : public UObject
{
    GENERATED_BODY()
public:
    UPROPERTY() UActivationBaseProbeWidget* Widget = nullptr;
    void NativeActivated();
    void NativeDeactivated();
    UFUNCTION() void DynamicActivated();
    UFUNCTION() void DynamicDeactivated();
};
void RegisterActivationBaseProbeDriver();
