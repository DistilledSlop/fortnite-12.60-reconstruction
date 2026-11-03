// DRAFT - UNCOMPILED, UNCOUNTED. Source: native 0x14160c6f0 (called by OnRep wrapper 0x142781830 with this+0x14d8 = RepSharedAnimInfo).
// Reflected binding: UPROPERTY(ReplicatedUsing=OnRep_SharedAnim) FGameplayAbilityRepSharedAnim RepSharedAnimInfo (header lines 33-42):
//   AnimState @+0 (EFortSharedAnimationState), MontageSectionToPlay @+1 (uint8), AnimMontage @+8 (UAnimMontage*).
// Wrapper-only tail (0x142781830, NOT part of this body; belongs to OnRep_SharedAnim itself after this helper): see README.
// Non-reflected members required on UFortAbilitySystemComponent (offsets from body):
//   FTimerHandle  SharedAnimTimerHandle;   // +0x14e8 (cleared at entry, never re-set here: each SetTimer below uses a LOCAL handle)
//   FSimpleDelegate SharedAnimEndDelegate; // +0x14f0 ptr / +0x14f8 size (FDelegateBase layout). UNRESOLVED exact delegate type/signature; slot 0x28 = IsBound/IsSafeToExecute, 0x50 = execute (no visible args), 0x38 = destroy.
// Unresolved effect declarations (exact address; NO bodies, NO guesses):
//   Ability99::SpawnSoundLambda_World       vtable 0x145d1ff80/0x145d1ffd8  captures {UWorld*, USoundBase*, WeakMesh, Volume+Pitch(8B @Notify+0x40)}
//   Ability99::SpawnSoundLambda_Attached    vtable 0x145d1fed0/0x145d1ff28  captures {USoundBase*, WeakMesh, AttachName(8B @Notify+0x4c)}
//   Ability99::FeedbackLineLambda           vtable 0x145d20030/0x145d20088  captures {WeakPawn, WeakObj(FUN_1424ab210), Notify+0x38(8B), +0x40(8B), +0x48(8B), byte +0x50}
//   Ability99::EmoteSoundStateLambda        vtable 0x145d20190/0x145d201e8  captures {WeakASC(this), WeakState, WeakMesh, Duration, state+0x38 sound}
//   Ability99::GetFeedbackLineTarget(AActor*)  0x1424ab210 (Avatar->vslot 0x150 -> +0x130 ->(cast class 0x142927a60)-> +0x3a8)
//   Ability99::PawnEmoteHook(UObject* PawnObj7a8, float 0.5f, 0, 0)   vslot 0x528 on *(AFortPawn+0x7a8); guard in wrapper uses vslot 0x540

namespace Ability99
{
	// Lambda factories: each returns the FTimerDelegate whose body lives at the vtable above. DECLARATION ONLY.
	FTimerDelegate MakeSpawnSoundWorldDelegate(UWorld* World, USoundBase* Sound, TWeakObjectPtr<USceneComponent> Mesh, float Volume, float Pitch);       // 0x145d1ff80
	FTimerDelegate MakeSpawnSoundAttachedDelegate(USoundBase* Sound, TWeakObjectPtr<USceneComponent> Mesh, FName AttachName);                              // 0x145d1fed0
	FTimerDelegate MakeFeedbackLineDelegate(TWeakObjectPtr<AFortPawn> Pawn, TWeakObjectPtr<UObject> Target, const UObject* Notify);                         // 0x145d20030
	FTimerDelegate MakeEmoteSoundStateDelegate(UFortAbilitySystemComponent* ASC, UObject* State, TWeakObjectPtr<USceneComponent> Mesh, float Duration);    // 0x145d20190
	UObject* GetFeedbackLineTarget(AActor* Avatar);   // 0x1424ab210
	void     PawnEmoteHook(UObject* PawnObject, float Arg, int32 A, int32 B); // vslot 0x528
}

void UFortAbilitySystemComponent::OnRep_SharedAnim_Body(const FGameplayAbilityRepSharedAnim& NewInfo /* == RepSharedAnimInfo, this+0x14d8 */)
{
	// 0x1440b4920: component owner's NetMode; early out only when it equals 1 (NM_DedicatedServer)
	if (GetNetMode() == NM_DedicatedServer)
	{
		return;
	}

	UWorld* World = GetWorld();                                // 0x1440b3260 (Owner->GetWorld(), else Outer world)
	FTimerManager& TimerManager = World->GetTimerManager();   // GameInstance (world+0x188)->TimerManager(+0xc0) else world+0x320

	if (SharedAnimTimerHandle.IsValid())                       // *(this+0x14e8) != 0
	{
		SharedAnimEndDelegate.ExecuteIfBound();                // 0x14f0: IsBound check (vslot 0x28) then vslot 0x50
		SharedAnimEndDelegate.Unbind();                        // vslot 0x38 + free, size=0
		if (TimerManager.TimerExists(SharedAnimTimerHandle))   // 0x144621fb0
		{
			TimerManager.ClearTimer(SharedAnimTimerHandle);    // FTimerManager_InternalClearTimer 0x144627760
		}
		SharedAnimTimerHandle.Invalidate();
	}

	UAnimMontage* Montage = NewInfo.AnimMontage;               // param_2+8
	if (Montage == nullptr)
	{
		return;
	}

	float RangeStart = 0.f, RangeEnd = 0.f;
	Montage->GetSectionStartAndEndTime(NewInfo.MontageSectionToPlay, RangeStart, RangeEnd);   // 0x143fcdbc0

	for (FAnimNotifyEvent& Event : Montage->Notifies)          // +0x80 data, +0x88 num, stride 0xB8
	{
		if (Event.Notify == nullptr && Event.NotifyStateClass == nullptr)   // +0x48 / +0x50
		{
			continue;
		}

		const float TriggerTime = Event.GetTriggerTime();      // 0x144020190
		if (!(RangeStart <= TriggerTime && TriggerTime <= RangeEnd))
		{
			continue;
		}
		const float Delay = TriggerTime - RangeStart;

		AActor* Avatar = AvatarActor;                          // 0x1406c7e50 returns *(this+0x3e0); offset match to Engine AvatarActor not byte-verified

		// --- 1. UAnimNotify_PlaySound ---------------------------------------------------------------
		if (UAnimNotify_PlaySound* PlaySound = Cast<UAnimNotify_PlaySound>(Event.Notify))   // 0x1446c5b50
		{
			if (PlaySound->Sound != nullptr && !PlaySound->Sound->IsLooping() /*0x1445af240: dur >= 10000*/ && Avatar != nullptr)
			{
				// 0x1447e2590 = USkeletalMeshComponent; AActor vslot 0x610 (component-by-class lookup; slot name unverified)
				if (UActorComponent* Mesh = Avatar->GetComponentByClass(USkeletalMeshComponent::StaticClass()))
				{
					FTimerHandle LocalHandle;                  // result handle is discarded
					if (!PlaySound->bFollow)                   // byte@+0x48 bit0 == 0
					{
						FTimerDelegate D = Ability99::MakeSpawnSoundWorldDelegate(World, PlaySound->Sound, Cast<USceneComponent>(Mesh), PlaySound->VolumeMultiplier, PlaySound->PitchMultiplier);
						TimerManager.SetTimer(LocalHandle, D, Delay, /*bLoop*/ false, /*FirstDelay*/ -1.f);
					}
					else
					{
						FTimerDelegate D = Ability99::MakeSpawnSoundAttachedDelegate(PlaySound->Sound, Cast<USceneComponent>(Mesh), PlaySound->AttachName);
						TimerManager.SetTimer(LocalHandle, D, Delay, false, -1.f);
					}
				}
			}
		}

		// --- 2. UFortAnimNotify_PlayFeedbackLine -----------------------------------------------------
		if (UFortAnimNotify_PlayFeedbackLine* Feedback = Cast<UFortAnimNotify_PlayFeedbackLine>(Event.Notify))   // 0x1427b2770
		{
			if (Feedback->FeedbackName /* FName @+0x40 */ != NAME_None)
			{
				UObject* FeedbackTarget = Ability99::GetFeedbackLineTarget(Avatar);   // 0x1424ab210 (null-safe)
				AFortPawn* AvatarPawn = Cast<AFortPawn>(Avatar);                       // 0x142a174f0
				FTimerHandle LocalHandle;
				FTimerDelegate D = Ability99::MakeFeedbackLineDelegate(AvatarPawn, FeedbackTarget, Feedback);
				TimerManager.SetTimer(LocalHandle, D, Delay, false, -1.f);
			}
		}

		// --- 3. UFortAnimNotifyState_EmoteSound ------------------------------------------------------
		if (UFortAnimNotifyState_EmoteSound* EmoteState = Cast<UFortAnimNotifyState_EmoteSound>(Event.NotifyStateClass))   // 0x1427b6b50
		{
			if (EmoteState->Sound /* @+0x38 */ != nullptr && Avatar != nullptr)
			{
				if (UActorComponent* Mesh = Avatar->GetComponentByClass(USkeletalMeshComponent::StaticClass()))
				{
					if (AFortPawn* Pawn = Cast<AFortPawn>(AvatarActor))                // FUN_1406c7e50 again, 0x142a174f0
					{
						if (UObject* PawnObj = *(UObject**)((uint8*)Pawn + 0x7a8))       // UNRESOLVED field @pawn+0x7a8
						{
							Ability99::PawnEmoteHook(PawnObj, 0.5f, 0, 0);               // vslot 0x528
						}
					}
					const float Duration = Event.GetDuration();                          // 0x14401af30
					FTimerHandle LocalHandle;
					FTimerDelegate D = Ability99::MakeEmoteSoundStateDelegate(this, EmoteState, Cast<USceneComponent>(Mesh), Duration);
					TimerManager.SetTimer(LocalHandle, D, Delay, false, -1.f);
				}
			}
		}
	}
}
