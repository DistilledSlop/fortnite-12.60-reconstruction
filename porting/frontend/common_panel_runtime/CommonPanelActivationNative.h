#pragma once
#include "CoreMinimal.h"
#include "ECommonInputType.h"
class UCommonActivatablePanel;
// Local source name for original Panel single-cast callbacks at 3d0/3e0.
DECLARE_DELEGATE_OneParam(FCommonPanelOperationCallback, UCommonActivatablePanel*);
