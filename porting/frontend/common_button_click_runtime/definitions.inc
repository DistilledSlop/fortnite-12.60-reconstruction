#include "UCommonUISubsystem.h"
#include "Engine/GameInstance.h"

void UCommonButton::NativeOnClicked()
{
    BP_OnClicked();
    NativeClicked.Broadcast();
    if (OnButtonClicked.IsBound())
    {
        OnButtonClicked.Broadcast(this);
    }
    FString Name;
    FString Type;
    FString Context;
    if (NativeGetClickAnalytics(Name, Type, Context))
    {
        if (UGameInstance* GameInstance = GetGameInstance())
        {
            (void)GameInstance->GetSubsystem<UCommonUISubsystem>();
        }
    }
}

bool UCommonButton::NativeGetClickAnalytics(FString& OutName, FString& OutType, FString& OutContext)
{
    GetName(OutName);
    OutType = TEXT("None");
    OutContext = TEXT("None");
    return true;
}
