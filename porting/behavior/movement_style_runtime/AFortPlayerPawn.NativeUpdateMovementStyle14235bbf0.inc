// CONFIDENCE: high. Original 0x14235bbf0.
if (GetLocalRole() == ROLE_SimulatedProxy) return;
if (AbilitySystemComponent && AbilitySystemComponent->HasMatchingGameplayTag(
        FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x6b0)))
{
    NativeSetMovementStyle14234bf20(4);
    return;
}
// The original first movement-mode read requires an initialized component.
if (GetCharacterMovement()->MovementMode == MOVE_Flying)
{
    NativeSetMovementStyle14234bf20(5);
}
else if (bIsTargeting)
{
    NativeSetMovementStyle14234bf20(1);
}
else if (NativeSprintState1062Bit0 &&
         (!NativeRequireSprintTag1478de49c() || NativeSprintTagGate14232cb50()))
{
    NativeSetMovementStyle14234bf20(3);
}
else
{
    UFortMovementComp_Character* Movement = Cast<UFortMovementComp_Character>(GetCharacterMovement());
    NativeSetMovementStyle14234bf20(Movement && Movement->MovementMode == MOVE_Custom &&
        Movement->CustomMovementMode == 12 ? 5 : 0);
}
