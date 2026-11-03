#pragma once
#include "UObject/SoftObjectPtr.h"

class UAthenaHUDBase;
class UFortUIStateWidget_NUI;
UClass* NativeLoadHUDClass144c7d620(const TSoftClassPtr<UAthenaHUDBase>& Class, bool bLogLoad);
UClass* NativeLoadStateClass144da33b0(const TSoftClassPtr<UFortUIStateWidget_NUI>& Class, bool bLogLoad);

// Shared context provider: UFortGameInstance::NativeGetGameMode141ed4e30(nullptr).
#include "UFortGameInstance.h"
