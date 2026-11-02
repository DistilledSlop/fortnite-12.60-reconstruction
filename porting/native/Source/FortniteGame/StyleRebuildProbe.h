#pragma once
#include "CoreMinimal.h"
#include "UCommonButton.h"
#include "UCommonButtonStyle.h"
#include "UCommonTextStyle.h"
#include "StyleRebuildProbe.generated.h"

UCLASS() class URebuildProbeStyleA : public UCommonButtonStyle { GENERATED_BODY() };
UCLASS() class URebuildProbeStyleB : public UCommonButtonStyle { GENERATED_BODY() };
UCLASS() class URebuildProbeText : public UCommonTextStyle { GENERATED_BODY() };

// Instrument the actual native virtual forwarder AND the real reflected event.
UCLASS()
class URebuildProbeButton : public UCommonButton
{
    GENERATED_BODY()
public:
    int32 NativeEvents = 0;
    int32 ReflectedEvents = 0;
    int32 ObservedFontSize = 0;
    bool bSwapAtEvent = false;
    bool bReenterAtEvent = false;
    bool bClearBrushColorsAtEvent = false;
    UCommonButtonInternal* ReplacementInternal = nullptr;
    virtual void ProcessEvent(UFunction* Function, void* Params) override;
protected:
    virtual void NativeOnCurrentTextStyleChanged() override;
    friend struct FCommonButtonStyleRebuildProbeAccess;
};

// Caller must have a real initialized engine/game thread and Slate application.
// Returns false on a failed actual-object/Slate observation; emits one named row.
bool RunStyleRebuildProbe(TFunctionRef<void(const TCHAR*, bool)> Record);
