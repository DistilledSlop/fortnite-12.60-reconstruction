#pragma once
#include "CoreMinimal.h"
#include "UCommonButton.h"
#include "BaseClickProbe.generated.h"

UCLASS()
class UBaseClickProbeButton : public UCommonButton
{
    GENERATED_BODY()
public:
    TArray<FString> Log;
    TWeakObjectPtr<UWorld> ProbeWorld;
    mutable int32 WorldCalls = 0;
    bool bAnalytics = false;
    bool bReenter = false;
    bool bInside = false;
    bool bStopDouble = false;
    bool bReenterSelection = false;
    virtual UWorld* GetWorld() const override;
    virtual void ProcessEvent(UFunction* Function, void* Parameters) override;
protected:
    virtual void NativeOnCurrentTextStyleChanged() override;
    virtual bool NativeGetClickAnalytics(FString& A, FString& B, FString& C) override;
};

UCLASS()
class UBaseClickProbeReceiver : public UObject
{
    GENERATED_BODY()
public:
    UBaseClickProbeButton* Button = nullptr; // fixture roots Button separately
    UFUNCTION() void OnDynamic(UCommonButton* Clicked);
    UFUNCTION() void OnDoubleDynamic(UCommonButton* Clicked);
    UFUNCTION() void OnSelectionDynamic(UCommonButton* SelectedButton, bool Selected);
    void OnNative();
};

bool RunBaseClickProbe(UWorld* World, TFunctionRef<void(const TCHAR*, bool)> Record);
