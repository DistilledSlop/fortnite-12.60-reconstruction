void UFortRootViewportLayout::NativeSetDormant14516e780(bool bDormant)
{
	if (NativeDormant2aa != bDormant)
	{
		ULocalPlayer* Player = GetOwningLocalPlayer();
		const int32 ControllerId = Player ? Player->GetControllerId() : -1;
		const TCHAR* Primary = Player && Player->IsPrimaryPlayer() ? TEXT("[Primary]") : TEXT("[Non-Primary]");
		UE_LOG(LogFortUI, Display, TEXT("%s RootViewportLayout Dormancy changed for [%d] from [%s] to [%s]"),
			Primary, ControllerId,
			NativeDormant2aa ? TEXT("Dormant") : TEXT("Not-Dormant"),
			bDormant ? TEXT("Dormant") : TEXT("Not-Dormant"));
		NativeDormant2aa = bDormant;
		NativeDormancyChanged488();
	}
}

// RebuildWidget produces these real shared widgets; no replacement Slate tree.
void UFortRootViewportLayout::NativeSetPresentation145152480(uint8 Presentation)
{
	if (NativePresentationWidget2d8.IsValid())
	{
		if (Presentation == 2)
		{
			NativePresentationWidget2d8->SetVisibility(EVisibility::Collapsed);
			NativePresentationBox2b8->SetPadding(FMargin(0.0f));
			NativePresentationScale2c8->SetStretch(EStretch::None);
		}
		else
		{
			NativePresentationWidget2d8->SetVisibility(EVisibility::SelfHitTestInvisible);
			NativePresentationBox2b8->SetPadding(FMargin(GFortUILetoBorderWidth));
			NativePresentationScale2c8->SetStretch(EStretch::UserSpecified);
		}
	}
}

// Base implementation only. Athena overrides this with1451660f0, which must run.
void UFortRootViewportLayout::NativeDormancyChanged488()
{
	if (UFortLocalPlayer* Player = Cast<UFortLocalPlayer>(GetOwningLocalPlayer()))
	{
		Player->NativeRootAwake341 = !NativeDormant2aa;
	}
	SetVisibility(NativeDormant2aa ? ESlateVisibility::Collapsed : ESlateVisibility::SelfHitTestInvisible);
	if (ULocalPlayer* Player = GetOwningLocalPlayer())
	{
		if (UCommonUIActionRouter* Router = Player->GetSubsystem<UCommonUIActionRouter>())
		{
			Router->NativeSetRootGate1407f78c0(!NativeDormant2aa);
		}
	}
	NativeDormancyChanged2f8.Broadcast(NativeDormant2aa);
}
