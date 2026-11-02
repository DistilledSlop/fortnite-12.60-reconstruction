TSharedRef<SWidget> UFortRootViewportLayout::RebuildWidget()
{
	TSharedRef<SBox> RootBox = SAssignNew(NativeRootBox2e8, SBox);
	TSharedRef<SWidget> UserContent = Super::RebuildWidget();
	TSharedRef<SWidget> RootSurface =
		SNew(SOverlay)
		+ SOverlay::Slot()[RootBox]
		+ SOverlay::Slot()[UserContent];

	ULocalPlayer* Player = GetOwningLocalPlayer();
	if (Player && !Player->IsPrimaryPlayer() && LetoLayoutBorderClass && GFortUIEnableLetoBorder)
	{
		UFortLetoLayoutBorder* Border = CreateWidget<UFortLetoLayoutBorder>(this, LetoLayoutBorderClass);
		NativePresentationWidget2d8 = Border->TakeWidget();
		TSharedRef<SWidget> ScaledContent =
			SAssignNew(NativePresentationScale2c8, SScaleBox)
			.HAlign(HAlign_Fill)
			.VAlign(VAlign_Fill)
			.UserSpecifiedScale(GFortUILetoBorderedContentScale)
			.Visibility(EVisibility::SelfHitTestInvisible)
			[RootSurface];
		TSharedRef<SWidget> PaddedContent =
			SAssignNew(NativePresentationBox2b8, SBox)
			[ScaledContent];
		return SNew(SOverlay)
			+ SOverlay::Slot()[PaddedContent]
			+ SOverlay::Slot()[NativePresentationWidget2d8.ToSharedRef()];
	}

	return RootSurface;
}
