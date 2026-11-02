#pragma once
#include "UObject/SoftObjectPtr.h"

class UAthenaHUDBase;
class UFortUIStateWidget_NUI;
UClass* NativeLoadHUDClass144c7d620(const TSoftClassPtr<UAthenaHUDBase>& Class, bool bLogLoad);
UClass* NativeLoadStateClass144da33b0(const TSoftClassPtr<UFortUIStateWidget_NUI>& Class, bool bLogLoad);

// Unclosed provider: 141ed4e30 with null world context. Its zero/nonzero
// result controls the selector-4 campaign override.
uint8 NativeGetGameMode141ed4e30();
