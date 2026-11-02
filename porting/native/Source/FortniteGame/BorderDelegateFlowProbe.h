#pragma once
#include "UCommonBorder.h"
#include "UCommonBorderStyle.h"
#include "BorderDelegateFlowProbe.generated.h"

UCLASS() class UBorderFlowStyle : public UCommonBorderStyle { GENERATED_BODY() };
UCLASS() class UBorderFlowWidget : public UCommonBorder
{
    GENERATED_BODY()
public:
    int32 Syncs = 0;
    virtual void SynchronizeProperties() override { ++Syncs; Super::SynchronizeProperties(); }
    TSharedPtr<SBorder> Border() const { return MyBorder; }
};
void RegisterBorderDelegateFlowProbeDriver();
