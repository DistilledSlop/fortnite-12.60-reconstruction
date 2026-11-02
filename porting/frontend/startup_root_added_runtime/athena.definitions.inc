// 1451660f0. Super's effects are inlined in the original; their ordering remains.
void UFortRootViewportLayout_Athena::NativeDormancyChanged488()
{
	Super::NativeDormancyChanged488();
	NativeRefreshLetoDormancy14516be20();
	UFortLocalPlayer* Player = Cast<UFortLocalPlayer>(GetOwningLocalPlayer());
	if (!Player || NativeDormant2aa || Player->IsPrimaryPlayer() || Player->NativePredicate14238f060())
	{
		return;
	}
	if (NativeGetManager14514e410(this)->NativeCurrentState6c != FStartupRootNativeTags::Get().StartupLogin)
	{
		return;
	}
	UClass* LoginClass = NativeLoadLoginClass145141970(UFortUIDataConfiguration::NativeGet144db86e0()->StartupStateWidget_Login, true);
	if (LoginClass && LoginClass->IsChildOf(UFortUIStateWidget_Login::StaticClass()) &&
		LoginClass->IsChildOf(UCommonActivatableWidget::StaticClass()))
	{
		// Actual original initializer target1406801c0 is ret0: it intentionally
		// performs no initialization. The container acquisition/activation is real.
		UFortUIStateWidget_Login* Login = Cast<UFortUIStateWidget_Login>(
			WidgetQueue_ModalContent->NativeAddWidget1407aa4c0(LoginClass, [](UCommonActivatableWidget&) {}));
		if (Login)
		{
			Login->NativeDeactivated2a8.AddWeakLambda(this,
				[this, Login]() { NativeLoginDeactivated1451529d0(Login); });
			return;
		}
	}
	NativeLoginDeactivated1451529d0(nullptr);
}

// 145167a40. Legacy selectors are exact numeric inputs to14516f380; no original
// source enum names have been inferred. Every class producer remains typed.
void UFortRootViewportLayout_Athena::NativeStateChanged490(const FGameplayTag& State)
{
	Super::NativeStateChanged490(State);
	NativeRefreshLetoDormancy14516be20();
	UFortLocalPlayer* Player = Cast<UFortLocalPlayer>(GetOwningLocalPlayer());
	// The original calls IsPrimaryPlayer on this result without a null fallback.
	if (!Player->IsPrimaryPlayer() && !Player->NativePredicate14238f060())
	{
		return;
	}
	const auto& Tags = FStartupRootNativeTags::Get();
	uint8 LegacySelector = 0;
	UClass* ChosenClass = nullptr;
	if (State.MatchesTag(Tags.Startup))
	{
		FGameplayTag StartupState = State.MatchesTag(Tags.Startup) ? State : FGameplayTag();
		if (StartupState == Tags.StartupLogin) LegacySelector = 1;
		else if (StartupState == Tags.StartupSubgameSelect) LegacySelector = 3;
		else if (StartupState == Tags.StartupRejoin) LegacySelector = 2;
		if (StartupState.MatchesTag(Tags.StartupLogin))
		{
			ChosenClass = NativeLoadLoginClass145141970(UFortUIDataConfiguration::NativeGet144db86e0()->StartupStateWidget_Login, true);
			if (ChosenClass && !ChosenClass->IsChildOf(UFortUIStateWidget_Login::StaticClass())) ChosenClass = nullptr;
		}
		else if (StartupState.MatchesTag(Tags.StartupSubgameSelect))
		{
			ChosenClass = NativeLoadSubgameClass145141b80(UFortUIDataConfiguration::NativeGet144db86e0()->StartupStateWidget_SubgameSelect, true);
			if (ChosenClass && !ChosenClass->IsChildOf(UStartUpUIStateWidget_SubgameSelect::StaticClass())) ChosenClass = nullptr;
		}
	}
	else if (State.MatchesTag(Tags.Athena))
	{
		FGameplayTag AthenaState = State.MatchesTag(Tags.Athena) ? State : FGameplayTag();
		if (AthenaState.MatchesTag(Tags.AthenaFrontend))
		{
			ChosenClass = NativeLoadFrontendClass145141760(UFortUIDataConfiguration::NativeGet144db86e0()->AthenaStateWidget_Frontend, true);
			if (ChosenClass && !ChosenClass->IsChildOf(UAthenaUIStateWidget_Frontend::StaticClass())) ChosenClass = nullptr;
		}
		else if (AthenaState.MatchesTag(Tags.AthenaBattleRoyale))
		{
			ChosenClass = UFortUIDataConfiguration::NativeGet144db86e0()->NativeGetHUDClass268();
			if (ChosenClass && !ChosenClass->IsChildOf(UAthenaHUDBase::StaticClass())) ChosenClass = nullptr;
		}
		else if (AthenaState.MatchesTag(Tags.AthenaSpectator) || AthenaState.MatchesTag(Tags.AthenaReplay))
		{
			ChosenClass = NativeLoadReplayClass145141550(UFortUIDataConfiguration::NativeGet144db86e0()->AthenaStateWidget_Replay, true);
			if (ChosenClass && !ChosenClass->IsChildOf(UAthenaBroadcastHUDBase::StaticClass())) ChosenClass = nullptr;
		}
		else if (AthenaState == Tags.AthenaFrontend) LegacySelector = 4;
		else if (AthenaState == Tags.AthenaBattleRoyale) LegacySelector = 9;
		else if (AthenaState == Tags.AthenaSpectator) LegacySelector = 10;
		else if (AthenaState == Tags.AthenaReplay) LegacySelector = 11;
		else if (AthenaState == Tags.AthenaDevCustomGame) LegacySelector = 5;
		else if (AthenaState == Tags.AthenaRejoin) LegacySelector = 2;
	}
	if ((!ChosenClass || !ChosenClass->IsChildOf(UFortUIStateWidgetBase::StaticClass())) && State.IsValid())
	{
		NativeSetLegacyState14516f380(LegacySelector);
		return;
	}
	NativeSetLegacyState14516f380(0);
	UFortUIStateWidgetBase* NewState = nullptr;
	if (ChosenClass && ChosenClass->IsChildOf(UFortUIStateWidgetBase::StaticClass()))
	{
		NewState = Cast<UFortUIStateWidgetBase>(CreateWidget<UFortUIStateWidgetBase>(this, ChosenClass, NAME_None));
	}
	UFortUIStateWidgetBase* PreviousState = CurrentStateWidget;
	UE_LOG(LogFortUI, Log, TEXT("Replacing state widget [%s] with [%s]"),
		PreviousState ? *PreviousState->GetFName().ToString() : TEXT("N/A"),
		NewState ? *NewState->GetFName().ToString() : TEXT("N/A"));
	if (PreviousState) PreviousState->DeactivateWidget();
	NativeClearLayers498();
	CurrentStateWidget = NewState;
	NativeAttachState145174c00(NewState);
	if (NewState)
	{
		NewState->ActivateWidget();
		CurrentLegacyStateWidget = nullptr;
	}
	NativeRemoveWidget145174fa0(PreviousState);
	if (NotificationManager)
	{
		if (UFortUINotificationQueue* Queue = NotificationManager->NativeGetQueue144e00700(0))
		{
			ToastDisplayArea_ToastHandler->NativeSetQueue144fb3e40(Queue);
		}
	}
}

// Captured continuation installed by Athena488 on the Login widget's real
// native no-argument deactivation multicast. Also called with null on failure.
void UFortRootViewportLayout_Athena::NativeLoginDeactivated1451529d0(UFortUIStateWidget_Login* Login)
{
	if (Login)
	{
		Login->NativeDeactivated2a8.RemoveAll(this);
		WidgetQueue_ModalContent->NativeRemoveWidget1407f1000(Login);
	}
	NativeSetDormant14516e780(true);
	// The original obtains GI through this widget's world and dereferences it
	// for first-player lookup; do not introduce a missing-GI fallback.
	if (UFortRootViewportLayout* FirstRoot = Cast<UFortRootViewportLayout>(
		NativeGetRoot14514e760(GetGameInstance()->GetFirstGamePlayer())))
	{
		FirstRoot->NativeSetDormant14516e780(false);
	}
}
