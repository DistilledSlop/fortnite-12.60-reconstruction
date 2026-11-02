// 141f3ad70, source alias. Requires Misc/CommandLine.h and Misc/Parse.h.
static bool NativeNoMcp141f3ad70()
{
	static const bool bNoMcp = FParse::Param(FCommandLine::Get(), TEXT("nomcp"));
	return bNoMcp;
}

// Source alias for FortUIDirector primary virtual +0x280, body 145146b80.
void UFortUIDirector::NativeAddLocalPlayer(UFortLocalPlayer* LocalPlayer)
{
	NativeUpdateManager145176990();
	if (LocalPlayer && CurrentManager)
	{
		CurrentManager->NativeAddLocalPlayer145146bc0(LocalPlayer);
	}
}

// 145176990. Property names checked against reflection block 14700d9a0.
void UFortUIDirector::NativeUpdateManager145176990()
{
	const auto IsManagerClass = [](UClass* Class)
	{
		return Class && Class->IsChildOf(UFortUIManager::StaticClass());
	};
	if (!IsManagerClass(EntrywayManagerClass.Get()) ||
		!IsManagerClass(AthenaManagerClass.Get()) ||
		!IsManagerClass(CampaignManagerClass.Get()))
	{
		// 142eda260 is FSoftObjectPath::TryLoad, including its UClass cast.
		EntrywayManagerClass = Cast<UClass>(EntrywayManagerClassPath.TryLoad());
		AthenaManagerClass = Cast<UClass>(AthenaManagerClassPath.TryLoad());
		CampaignManagerClass = Cast<UClass>(CampaignManagerClassPath.TryLoad());
	}
	UClass* Previous = CurrentManager ? CurrentManager->GetClass() : nullptr;
	UGameInstance* GameInstance = GetGameInstance();
	UFortLocalPlayer* FirstPlayer = Cast<UFortLocalPlayer>(GameInstance->GetFirstGamePlayer());
	UClass* Selected = nullptr;
	if (FirstPlayer)
	{
		// 141f3ad70 caches this command-line predicate on first invocation.
		// Keep it lazy: the original does not evaluate nomcp on the true branch.
		if (!FirstPlayer->NativePredicate14238f060() && !NativeNoMcp141f3ad70())
		{
			Selected = EntrywayManagerClass.Get();
		}
		else
		{
			switch (static_cast<UFortGameInstance*>(GameInstance)->NativeGetSubGame141e5f160())
			{
			case 2: Selected = EntrywayManagerClass.Get(); break;
			case 1: Selected = AthenaManagerClass.Get(); break;
			case 0: Selected = CampaignManagerClass.Get(); break;
			default: break;
			}
		}
	}
	UClass* ValidPrevious = IsManagerClass(Previous) ? Previous : nullptr;
	UClass* ValidSelected = IsManagerClass(Selected) ? Selected : nullptr;
	if (ValidPrevious != ValidSelected &&
		!static_cast<UFortGameInstance*>(GameInstance)->NativeIsUIManagerChangeBlocked7bb())
	{
		// Deliberately pass Selected, not ValidSelected: the callee validates again.
		NativeSetManager1451746d0(Selected);
	}
}
