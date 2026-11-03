#pragma once
#include "CoreMinimal.h"
class UCommonActivatablePanel;
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonLegacyPanelChanged, UCommonActivatablePanel*);
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonLegacyInputSuspensionChanged, bool);
