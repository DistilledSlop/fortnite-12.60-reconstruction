// Address-bearing names are source aliases, not recovered original symbols.
// Root has the Athena LayoutClass invariant also required by reflected PlayerWidgets.
void UFortUIManager_Startup::NativeOnRootAdded298(UFortLocalPlayer* LocalPlayer, UUserWidget* Root)
{
	if (UFortRootViewportLayout* StateRoot = Cast<UFortRootViewportLayout>(Root))
	{
		StateRoot->NativeStateChanged490(NativeCurrentState6c);
	}
	auto* AthenaRoot = static_cast<UFortRootViewportLayout_Athena*>(Root);
	PlayerWidgets.Add(LocalPlayer, AthenaRoot);
	uint8 Presentation = 0;
	if (NativeCurrentState6c.IsValid())
	{
		if (NativeCurrentState6c.MatchesTag(FStartupRootNativeTags::Get().AthenaFrontend))
		{
			Presentation = 1;
		}
		else if (NativeCurrentState6c == FStartupRootNativeTags::Get().AthenaReplay)
		{
			Presentation = 2;
		}
	}
	AthenaRoot->NativeSetDormant14516e780(!LocalPlayer->IsPrimaryPlayer() && Presentation != 2);
	AthenaRoot->NativeSetPresentation145152480(Presentation);
	GetTypedOuter<UFortGameInstance>()->NativeRootLayoutChanged5d8.Broadcast();
}
