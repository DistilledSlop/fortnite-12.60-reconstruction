// CONFIDENCE: HIGH (source recovery; build/runtime validation pending).
// Native 0x141de18c0.
// Signature: void AFortWindManager::NativeUpdateWindAudio(float DeltaSeconds)
if (GetNetMode() == NM_DedicatedServer)
	return;
UWorld* World = GetWorld();
if (!World)
	return;

ABuildingSMActor* const PreviousActor = ActiveAudioWindResponder.WindUpdatingBuildingSMActor;
if (!NativeIsWindEnabled())
{
	FFortWindResponder DefaultResponder;
	DefaultResponder.WindUpdatingBuildingSMActor = nullptr;
	DefaultResponder.WindSpeedCurve = nullptr;
	DefaultResponder.WindPannerSpeedCurve = nullptr;
	DefaultResponder.WindAudio = nullptr;
	DefaultResponder.MaterialParameterPairIndices = 0;
	DefaultResponder.WindSpeed = 0.0f;
	DefaultResponder.bHasSetupAnimatingMaterials = false;
	// Native leaves bounds and effective-distance storage unspecified for this
	// null-actor responder; its TArrays default-construct empty.
	ActiveAudioWindResponder = MoveTemp(DefaultResponder);
}
else
{
	FVector ViewerPosition = FVector::ZeroVector;
	if (AFortPlayerController* Controller = Cast<AFortPlayerController>(World->GetFirstPlayerController()))
	{
		if (Controller->PlayerCameraManager)
			ViewerPosition = Controller->PlayerCameraManager->GetCameraLocation();
	}
	NativeRefreshNearbyCache(ViewerPosition);
	if (bAllowResponderAudio)
	{
		float BestDistanceSquared = AudioWindMaxResponderDistance * AudioWindMaxResponderDistance;
		FFortWindResponder* BestResponder = nullptr;
		for (int32 Index : NearbyCacheIndicesForAudio)
		{
			if (!WindResponders.IsValidIndex(Index))
				continue;
			FFortWindResponder& Candidate = WindResponders[Index];
			if (!Candidate.WindUpdatingBuildingSMActor || !Candidate.WindAudio || !(Candidate.WindSpeed > 0.0f))
				continue;

			float Bounds[6];
			FMemory::Memcpy(Bounds, Candidate.NativeBounds_0x68, sizeof(Bounds));
			float DistanceSquared = 0.0f;
			float Boundary = Bounds[0];
			if (ViewerPosition.X < Boundary || (Boundary = Bounds[3], Boundary < ViewerPosition.X))
				DistanceSquared = (ViewerPosition.X - Boundary) * (ViewerPosition.X - Boundary);
			Boundary = Bounds[1];
			if (ViewerPosition.Y < Boundary || (Boundary = Bounds[4], Boundary < ViewerPosition.Y))
				DistanceSquared += (ViewerPosition.Y - Boundary) * (ViewerPosition.Y - Boundary);
			Boundary = Bounds[2];
			if (ViewerPosition.Z < Boundary || (Boundary = Bounds[5], Boundary < ViewerPosition.Z))
				DistanceSquared += (ViewerPosition.Z - Boundary) * (ViewerPosition.Z - Boundary);
			if (DistanceSquared < BestDistanceSquared)
			{
				BestResponder = &Candidate;
				BestDistanceSquared = DistanceSquared;
			}
		}

		if (!BestResponder)
		{
			FFortWindResponder DefaultResponder;
			DefaultResponder.WindUpdatingBuildingSMActor = nullptr;
			DefaultResponder.WindSpeedCurve = nullptr;
			DefaultResponder.WindPannerSpeedCurve = nullptr;
			DefaultResponder.WindAudio = nullptr;
			DefaultResponder.MaterialParameterPairIndices = 0;
			DefaultResponder.WindSpeed = 0.0f;
			DefaultResponder.bHasSetupAnimatingMaterials = false;
			// Native leaves bounds and effective-distance storage unspecified for this
			// null-actor responder; its TArrays default-construct empty.
			ActiveAudioWindResponder = MoveTemp(DefaultResponder);
		}
		else if (BestResponder->WindUpdatingBuildingSMActor != ActiveAudioWindResponder.WindUpdatingBuildingSMActor)
		{
			ActiveAudioWindResponder = *BestResponder;
			UAudioComponent* const* Existing = WindActorToWindResponderAudioMap.Find(ActiveAudioWindResponder.WindUpdatingBuildingSMActor);
			if (!Existing || !*Existing)
			{
				UAudioComponent* Component = nullptr;
				if (WindAudioComponentPool.Num() > 0)
				{
					Component = WindAudioComponentPool.Pop(true);
					if (Component && !Component->IsPendingKill())
						Component->Rename(nullptr, ActiveAudioWindResponder.WindUpdatingBuildingSMActor, static_cast<ERenameFlags>(0x15));
					else
						Component = nullptr;
				}
				if (!Component)
					Component = NewObject<UAudioComponent>(ActiveAudioWindResponder.WindUpdatingBuildingSMActor, NAME_None, RF_Transient | RF_TextExportTransient);
				Component->SetupAttachment(ActiveAudioWindResponder.WindUpdatingBuildingSMActor->GetRootComponent(), NAME_None);
				FVector BoundsOrigin;
				FVector BoundsExtent;
				ActiveAudioWindResponder.WindUpdatingBuildingSMActor->GetActorBounds(false, BoundsOrigin, BoundsExtent, false);
				USceneComponent* Root = ActiveAudioWindResponder.WindUpdatingBuildingSMActor->GetRootComponent();
				const FVector RootLocation = Root ? Root->GetComponentLocation() : FVector::ZeroVector;
				Component->SetRelativeLocation_Direct(BoundsOrigin - RootLocation);
				Component->Sound = ActiveAudioWindResponder.WindAudio;
				Component->VolumeMultiplier = 0.0f;
				Component->RegisterComponent();
				if (!Component->IsPlaying())
					Component->Play(0.0f);
				WindActorToWindResponderAudioMap.Add(ActiveAudioWindResponder.WindUpdatingBuildingSMActor, Component);
			}
		}
	}
}

if (PreviousActor != ActiveAudioWindResponder.WindUpdatingBuildingSMActor)
{
	if (UAudioComponent* const* Previous = WindActorToWindResponderAudioMap.Find(PreviousActor))
	{
		if (UAudioComponent* Component = *Previous)
			OnWindAudioBecameInactive(Component);
	}
	// The inactive event may change the active responder or mutate the map.
	if (UAudioComponent* const* Current = WindActorToWindResponderAudioMap.Find(ActiveAudioWindResponder.WindUpdatingBuildingSMActor))
	{
		if (UAudioComponent* Component = *Current)
			OnWindAudioBecameActive(Component);
	}
}

const float InterpSpeed = AudioWindInterpSpeed;
for (auto It = WindActorToWindResponderAudioMap.CreateIterator(); It; ++It)
{
	ABuildingSMActor* Actor = It.Key();
	UAudioComponent* Component = It.Value();
	if (!Actor || !Component)
	{
		It.RemoveCurrent();
		continue;
	}
	float Volume;
	if (Actor == ActiveAudioWindResponder.WindUpdatingBuildingSMActor)
	{
		Volume = Component->VolumeMultiplier + DeltaSeconds * InterpSpeed;
		Component->SetFloatParameter(AudioWindSpeedParameterName, ActiveAudioWindResponder.WindSpeed);
	}
	else
	{
		Volume = Component->VolumeMultiplier - DeltaSeconds * InterpSpeed;
	}

	// Keep the native comparison order: unordered volume becomes 1, and both
	// signed zeros take the stop branch. FMath::Clamp changes NaN behavior.
	bool bStop = Volume < 0.0f;
	if (!bStop)
	{
		if (Volume < 1.0f)
			bStop = Volume == 0.0f;
		else
			Volume = 1.0f;
	}
	if (bStop)
	{
		Component->Stop();
		if (!Component->IsPendingKill())
		{
			Component->UnregisterComponent();
			Component->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepRelative, EDetachmentRule::KeepRelative, EDetachmentRule::KeepRelative, false));
			RemoveOwnedComponent(Component);
			Component->Rename(nullptr, this, static_cast<ERenameFlags>(0x15));
			WindAudioComponentPool.Add(Component);
		}
		It.RemoveCurrent();
	}
	else if (Volume != Component->VolumeMultiplier)
	{
		Component->SetVolumeMultiplier(Volume);
	}
}
