// CONFIDENCE: MEDIUM. Chunk 01 port. See ABuildingGameplayActor__SetTouchComponentForAbilityDeliveryInfo.evidence.json for captures and notes.
// Original address: 0x142740f70 (exec). Body 0x141beb3e0.
// Moves the ability-delivery touch binding from the old RegisteredTouchComponent to PrimitiveComponent.
if (Role != ROLE_Authority)
{
	return;
}
if (bUseSimpleActorTouchSetupForAbilityBuckets || RegisteredTouchComponent == PrimitiveComponent)
{
	return;
}

// Unbind the old component's overlap delegates.
if (RegisteredTouchComponent)
{
	if (bBoundBeginOverlapOnOldTouch)
	{
		RegisteredTouchComponent->OnComponentBeginOverlap.RemoveDynamic(this, &ABuildingGameplayActor::OnRegisteredTouchComponentBeginOverlap);
		bBoundBeginOverlapOnOldTouch = false;
	}
	if (bBoundEndOverlapOnOldTouch)
	{
		RegisteredTouchComponent->OnComponentEndOverlap.RemoveDynamic(this, &ABuildingGameplayActor::OnRegisteredTouchComponentEndOverlap);
		bBoundEndOverlapOnOldTouch = false;
	}
}

RegisteredTouchComponent = PrimitiveComponent;
if (PrimitiveComponent == nullptr)
{
	return;
}

// Bind the new component and queue every actor that is already overlapping it.
PrimitiveComponent->OnComponentBeginOverlap.AddDynamic(this, &ABuildingGameplayActor::OnRegisteredTouchComponentBeginOverlap);
bBoundBeginOverlapOnNewTouch = true;
PrimitiveComponent->OnComponentEndOverlap.AddDynamic(this, &ABuildingGameplayActor::OnRegisteredTouchComponentEndOverlap);
bBoundEndOverlapOnNewTouch = true;

TArray<AActor*> OverlappingActors;
PrimitiveComponent->GetOverlappingActors(OverlappingActors);
for (AActor* OverlappingActor : OverlappingActors)
{
	if (OverlappingActor)
	{
		QueueTouchingActorForAbility(OverlappingActor);   // FUN_141be3360
	}
}
