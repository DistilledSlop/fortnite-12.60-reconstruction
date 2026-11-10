// CONFIDENCE: MEDIUM
// Original: 0x1423ad260 (virtual Implementation of ServerUpdateActorOptions). Callees not named here are described in the evidence file.
// Store per-actor options into the target actor's options component.
if (OptionsKeys.Num() != OptionsValues.Num())
{
	UE_LOG(LogFort, Warning, TEXT("PLAYER OPTIONS: Mismatched key/value counts for target actor: %s"), *GetNameSafe(OptionsTarget));
	return;
}
if (OptionsTarget)
{
	NativeEnsureOptionsTargetReady(OptionsTarget);
}
UActorOptionsComponent* OptionsComponent = OptionsTarget ? OptionsTarget->NativeGetOptionsComponent() : nullptr;
if (!OptionsComponent)
{
	UE_LOG(LogFort, Warning, TEXT("PLAYER OPTIONS: Missing component on target actor: %s"), *GetNameSafe(OptionsTarget));
	return;
}
TMap<FString, FString> Options;
for (int32 Index = 0; Index < OptionsKeys.Num(); ++Index)
{
	Options.Add(OptionsKeys[Index], OptionsValues[Index]);
}
OptionsComponent->SetOptions(Options);
