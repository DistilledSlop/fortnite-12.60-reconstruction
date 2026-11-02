#pragma once
#include "UCommonTextBlock.h"
#include "UCommonTextStyle.h"
#include "Widgets/Text/STextBlock.h"
#include "CommonTextBlockStyleProbe.generated.h"

UCLASS()
class UTextStyleProbeStyle : public UCommonTextStyle
{
    GENERATED_BODY()
};

// The fixture only observes virtual dispatch and state; it always chains the real implementation.
UCLASS()
class UTextStyleProbeBlock : public UCommonTextBlock
{
    GENERATED_BODY()
public:
    int32 SyncCount = 0;
    int32 VisibilityCount = 0;
    bool bCaptureVisibility = false;
    FSlateFontInfo FontAtVisibility;
    FMargin MarginAtVisibility;
    FText SlateTextAtVisibility;
    virtual void SynchronizeProperties() override { ++SyncCount; Super::SynchronizeProperties(); }
    virtual void SetVisibility(ESlateVisibility V) override
    {
        if (bCaptureVisibility) { ++VisibilityCount; FontAtVisibility = Font; MarginAtVisibility = Margin; SlateTextAtVisibility = SlateText(); }
        Super::SetVisibility(V);
    }
    FMargin ProbeMargin() const { return Margin; }
    float ProbeLineHeight() const { return LineHeightPercentage; }
    bool HasSlate() const { return MyTextBlock.IsValid(); }
    FText SlateText() const { return MyTextBlock.IsValid() ? MyTextBlock->GetText() : FText::GetEmpty(); }
    FVector2D SlateDesired() const { if (MyTextBlock.IsValid()) { MyTextBlock->SlatePrepass(1.0f); return MyTextBlock->GetDesiredSize(); } return FVector2D::ZeroVector; }
    UFUNCTION() FText BoundProbeText() const { return FText::FromString(TEXT("delegate text")); }
};

void RegisterCommonTextBlockStyleProbeDriver();
