// CONFIDENCE: HIGH
// Original: exec thunk 0x142756f00 dispatches to ABuildingSMActor virtual +0xa40 (real body 0x141c107b0).
// That body forwards to the component: UFortAbilitySystemComponent virtual +0x570 (0x1406d0a30).
// Same forward as AFortPawn::NetMulticast_InvokeGameplayCueExecuted_Implementation (ported). Null component: no-op.
if (UFortAbilitySystemComponent* ASC = Cast<UFortAbilitySystemComponent>(AbilitySystemComponent))
{
    ASC->NetMulticast_InvokeGameplayCueExecuted_Implementation(GameplayCueTag, PredictionKey, EffectContext);
}
