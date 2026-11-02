#pragma once

#include "CommonActionBindingData.h"

// Production data providers recovered from the original binding constructor.
// Registration validates the descriptor before construction. Legacy owners must
// be widgets with a live local player, action router and Common Input subsystem;
// the selected gamepad byte must index an existing legacy gamepad entry.
class COMMONUI_API FCommonActionBindingEngineResolver final : public ICommonActionBindingDataResolver
{
public:
    const FUIInputAction& ResolveTag(FName Tag) override;
    const FCommonInputActionData& ResolveRow(const FDataTableRowHandle& Row) override;
    int32 ResolveGamepadIndex(UObject* Owner) override;
};
