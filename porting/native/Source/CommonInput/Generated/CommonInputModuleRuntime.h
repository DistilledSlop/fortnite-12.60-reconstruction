#pragma once

#include "CoreMinimal.h"

class UCommonInputSettings;

DECLARE_LOG_CATEGORY_EXTERN(LogCommonInput, Log, All);

COMMONINPUT_API UCommonInputSettings* GetCommonInputSettings();

COMMONINPUT_API bool ShouldShowCommonInputKeys();
