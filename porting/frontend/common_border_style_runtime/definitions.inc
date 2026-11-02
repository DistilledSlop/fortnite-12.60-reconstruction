// Candidate: requires the declarations and Slate includes in HANDOFF.md.
// Receiver 0x1407f9c50; synchronization 0x1407fdf60.
UCommonBorder::UCommonBorder(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    Style = nullptr;
    bReducePaddingBySafezone = false;
    MinimumPadding = FMargin(0.0f);
    bShowEffectWhenDisabled = false;
    Visibility = ESlateVisibility::SelfHitTestInvisible;
}

void UCommonBorder::SetStyle(TSubclassOf<UCommonBorderStyle> InStyle)
{
    Style = InStyle;
    if (MyBorder.IsValid())
    {
        SynchronizeProperties();
    }
}

void UCommonBorder::SynchronizeProperties()
{
    if (UClass* StyleClass = Style.Get())
    {
        if (const UCommonBorderStyle* StyleObject = Cast<UCommonBorderStyle>(StyleClass->GetDefaultObject(false)))
        {
            Background = StyleObject->Background;
        }
    }
    Super::SynchronizeProperties();
    NativeUpdateSafeZonePadding();
}

void UCommonBorder::NativeUpdateSafeZonePadding()
{
    FMargin SafeMargin(0.0f);
    if (bReducePaddingBySafezone && MyBorder.IsValid())
    {
        TSharedPtr<SViewport> GameViewport = FSlateApplication::Get().GetGameViewport();
        if (GameViewport.IsValid())
        {
            TSharedPtr<ISlateViewport> Viewport = GameViewport->GetViewportInterface().Pin();
            if (Viewport.IsValid())
            {
                FSlateApplication::Get().GetSafeZoneSize(SafeMargin, FVector2D(Viewport->GetSize()));
            }
        }
        MyBorder->SetPadding(FMargin(
            FMath::Max(MinimumPadding.Left, Padding.Left - SafeMargin.Left),
            FMath::Max(MinimumPadding.Top, Padding.Top - SafeMargin.Top),
            FMath::Max(MinimumPadding.Right, Padding.Right - SafeMargin.Right),
            FMath::Max(MinimumPadding.Bottom, Padding.Bottom - SafeMargin.Bottom)));
    }
}
