#pragma once

#include "UCommonButton.h"
#include "UCommonButtonStyle.h"
#include "UCommonTextStyle.h"
#include "UCommonButtonInternal.h"
#include "CommonButtonTextStyleProbe.generated.h"

// These classes are diagnostic fixtures; no input or selection producer is implied.
UCLASS()
class UButtonTextProbeNormal : public UCommonTextStyle {
    GENERATED_BODY()
};
UCLASS()
class UButtonTextProbeNormalHover : public UCommonTextStyle {
    GENERATED_BODY()
};
UCLASS()
class UButtonTextProbeSelected : public UCommonTextStyle {
    GENERATED_BODY()
};
UCLASS()
class UButtonTextProbeSelectedHover : public UCommonTextStyle {
    GENERATED_BODY()
};
UCLASS()
class UButtonTextProbeDisabled : public UCommonTextStyle {
    GENERATED_BODY()
};
UCLASS()
class UButtonTextProbeStyle : public UCommonButtonStyle {
    GENERATED_BODY()
};
UCLASS()
class UButtonTextProbeInternal : public UCommonButtonInternal
{
    GENERATED_BODY()
public:
    mutable int32 HoverReads = 0;
    bool FirstHover = false;
    bool LaterHover = false;
    virtual bool IsHovered() const override { return HoverReads++ == 0 ? FirstHover : LaterHover; }
    void Hover(bool First, bool Later) { FirstHover = First; LaterHover = Later; HoverReads = 0; }
};
UCLASS()
class UButtonTextProbeWidget : public UCommonButton
{
    GENERATED_BODY()
};

void RegisterCommonButtonTextStyleProbeDriver();
