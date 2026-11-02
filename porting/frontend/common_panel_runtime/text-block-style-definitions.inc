#include "UCommonTextStyle.h"

UCommonTextBlock::UCommonTextBlock(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Style = nullptr;
    ScrollStyle = nullptr;
    bDisplayAllCaps = false;
    bAutoCollapseWithEmptyText = false;
    MobileFontSizeMultiplier = 1.0f;
}

void UCommonTextBlock::SetStyle(TSubclassOf<UCommonTextStyle> InStyle)
{
    Style = InStyle;
    SynchronizeProperties();
}

void UCommonTextBlock::SynchronizeProperties()
{
    UClass* Class = Style.Get();
    if (Class && Class->IsChildOf(UCommonTextStyle::StaticClass()))
    {
        const UCommonTextStyle* StyleCDO = Cast<UCommonTextStyle>(Class->GetDefaultObject());
        if (StyleCDO)
        {
            Font = StyleCDO->Font;
            StrikeBrush = StyleCDO->StrikeBrush;
            Margin = StyleCDO->Margin;
            LineHeightPercentage = StyleCDO->LineHeightPercentage;
            ColorAndOpacity = FSlateColor(StyleCDO->Color);
            if (StyleCDO->bUsesDropShadow)
            {
                ShadowOffset = StyleCDO->ShadowOffset;
                ShadowColorAndOpacity = StyleCDO->ShadowColor;
            }
            else
            {
                ShadowOffset = FVector2D::ZeroVector;
                ShadowColorAndOpacity = FLinearColor::Transparent;
            }
        }
    }
    if (bAutoCollapseWithEmptyText)
    {
        SetVisibility(Text.IsEmpty() ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
    }
    Super::SynchronizeProperties();
}
