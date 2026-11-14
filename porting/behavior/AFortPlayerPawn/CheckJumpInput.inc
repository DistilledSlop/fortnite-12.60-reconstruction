// CONFIDENCE: HIGH. Complete original 0x14230d2f0; build/runtime pending.
if (IsLocallyControlled() && !bClientUpdating)
{
    NativeSprintState1062Bit0 = NativeSprintTagGate14232cb50();
}
uint8 PreviousStyle;
if ((*NativeGetCurrentMovementStyle14231a980(&PreviousStyle) == 3) != NativeSprintState1062Bit0)
{
    NativeUpdateMovementStyle14235bbf0();
}
if (bPressedJump && GetCharacterMovement()->IsFalling())
{
    return;
}
if (NativeParachuteRequest1fc0 == 0 || !NativeCanDeployParachute14230bcb0())
{
    ACharacter::CheckJumpInput(DeltaTime);
    return;
}
UWorld* World = GetWorld();
const float Now = World ? World->GetTimeSeconds() : 0.0f;
if (NativeParachuteRequest1fc0 == 1)
{
    if (bIsParachuteOpen)
    {
        NativeParachuteRequest1fc0 = 0;
        return;
    }
    if (!bIsParachuteForcedOpen)
    {
        AFortPlayerControllerGameplay* GameplayController = Cast<AFortPlayerControllerGameplay>(Controller);
        if (GameplayController && GameplayController->NativeHasActiveTimer1423d9510())
        {
            return;
        }
        // Original deliberately calls GetWorld again; retain potential callback/reentry effects.
        if (NativeLimitParachuteDeployRate148157cfc() != 0)
        {
            UWorld* CooldownWorld = GetWorld();
            // COMISS/JC rejects unordered operands as well as elapsed < cooldown.
            if (CooldownWorld && !(CooldownWorld->GetTimeSeconds() - NativeParachuteLastTime264c >= ParachuteCooldownToOpen))
            {
                return;
            }
        }
    }
    NativeOpenParachute14233e6e0(false); // original virtual +0x1330, preserve overrides
    NativeParachuteLastTime264c = Now; // written even if deployment callback did not deploy
}
else if (NativeParachuteRequest1fc0 == 2 && NativeCanCloseParachute142331a80())
{
    NativeCloseParachute14230e850();
    NativeParachuteLastTime264c = Now; // likewise written after no-op close
}
