// Original address: 0x142314600
// CONFIDENCE: high
if (ZiplineState.bIsZiplining && (GetLocalRole() == ROLE_AutonomousProxy || GetLocalRole() == ROLE_Authority))
{
    if (bFromJump)
    {
        const float Delay = ZiplineJumpActivateDelay.GetValueAtLevel(0.0f);
        if (UWorld* World = GetWorld())
        {
            if (World->GetTimeSeconds() - ZiplineState.TimeZipliningBegan < Delay)
            {
                return;
            }
        }
    }
    ZiplineState.Zipline = nullptr;
    ZiplineState.bIsZiplining = false;
    ZiplineState.bJumped = bFromJump;
    ZiplineState.AuthoritativeValue++;
    if (bFromJump)
    {
        if (UWorld* World = GetWorld())
        {
            ZiplineState.TimeZipliningEndedFromJump = World->GetTimeSeconds();
        }
    }
    else
    {
        ZiplineState.TimeZipliningEndedFromJump = -1.0f;
    }
    if (GetCharacterMovement() && GetCharacterMovement()->MovementMode == MOVE_Custom && GetCharacterMovement()->CustomMovementMode == EFortCustomMovement::Ziplining)
    {
        GetCharacterMovement()->SetMovementMode(MOVE_Falling, 0);
    }
    if (GetLocalRole() == ROLE_AutonomousProxy)
    {
        ServerSendZiplineState(ZiplineState);
    }
}
