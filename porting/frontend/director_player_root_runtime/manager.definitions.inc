// Source aliases; this fragment requires the real dependencies in contract.json.
// 145146bc0: state refresh and weak callback registration precede array lookup.
void UFortUIManager::NativeAddLocalPlayer145146bc0(UFortLocalPlayer* LocalPlayer)
{
	NativeUpdateUIState1451755b0();
	LocalPlayer->NativePlayerControllerChanged348.AddWeakLambda(this,
		[this, LocalPlayer]() { NativePlayerControllerChanged14514cba0(LocalPlayer); });
	FFortRootViewportLayoutInfo* Info = RootViewportLayouts.FindByPredicate(
		[LocalPlayer](const FFortRootViewportLayoutInfo& Item) { return Item.LocalPlayer == LocalPlayer; });
	if (Info)
	{
		NativeAttachRoot145146470(LocalPlayer, Info->RootLayout);
		Info->bAttached = true;
	}
	else
	{
		NativeCreatePlayerRoot14514ab50(LocalPlayer);
	}
}

// 14514ab50: no class/result fallback or null-result early return exists here.
void UFortUIManager::NativeCreatePlayerRoot14514ab50(UFortLocalPlayer* LocalPlayer)
{
	APlayerController* Controller = LocalPlayer->GetPlayerController(GetWorld());
	if (!Controller)
	{
		return;
	}
	UUserWidget* Root = CreateWidget<UUserWidget>(Controller, LayoutClass.LoadSynchronous(), NAME_None);
	const int32 Index = RootViewportLayouts.AddUninitialized();
	FFortRootViewportLayoutInfo& Info = RootViewportLayouts[Index];
	Info.LocalPlayer = LocalPlayer;
	Info.RootLayout = Root;
	Info.bAttached = true;
	NativeAttachRoot145146470(LocalPlayer, Root);
}

// 145146470: logging elided; AddToPlayerScreen return value is ignored.
void UFortUIManager::NativeAttachRoot145146470(UFortLocalPlayer* LocalPlayer, UUserWidget* Root)
{
	Root->SetPlayerContext(FLocalPlayerContext(LocalPlayer));
	Root->AddToPlayerScreen(0);
	NativeOnRootAdded298(LocalPlayer, Root);
}

// 14514cba0: lambda body stored in the weak delegate installed at player+348.
void UFortUIManager::NativePlayerControllerChanged14514cba0(UFortLocalPlayer* LocalPlayer)
{
	NativeRemovePlayerRootScreen14516d0d0(LocalPlayer);
	FFortRootViewportLayoutInfo* Info = RootViewportLayouts.FindByPredicate(
		[LocalPlayer](const FFortRootViewportLayoutInfo& Item) { return Item.LocalPlayer == LocalPlayer; });
	if (Info)
	{
		NativeAttachRoot145146470(LocalPlayer, Info->RootLayout);
		Info->bAttached = true;
	}
	else if (!NativePredicate141ed92b0())
	{
		NativeCreatePlayerRoot14514ab50(LocalPlayer);
	}
}
