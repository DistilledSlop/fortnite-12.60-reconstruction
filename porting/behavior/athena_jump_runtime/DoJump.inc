// CONFIDENCE: HIGH. Complete original body; class lifecycle acceptance remains separate.
// COMPLETE TYPED BODY PROPOSAL. All invoked operations resolved; root binds accompanying declarations.
// Original UFortMovementComp_CharacterAthena::DoJump(bool), 0x1419cd830.
const uint8 PreviousFlag = NativeDoJumpFlag1dfd;
NativeDoJumpFlag1dfd = 1;
if (JumpZVelocity <= 0.0f) // COMISS(0,JumpZ), JC continues for positive AND NaN.
{
    NativeDoJumpFlag1dfd = PreviousFlag;
    return true; // Early return intentionally bypasses authority floor bookkeeping.
}

bool bSpecialJump = MovementMode == MOVE_Custom && CustomMovementMode == 10
    && !(GetWaterImmersionDepth() < 0.0f); // COMISS(0,depth), JA rejects negative only.
if (AFortPlayerPawn* Player = Cast<AFortPlayerPawn>(CharacterOwner))
{
    if (Player->bIsWaterJump) bSpecialJump = true;
}

bool bSuccess;
if (JumpPenalties.Num() == 0)
{
    // Even a special jump goes through Fort base on this path.
    bSuccess = Super::DoJump(bReplayingMoves);
}
else
{
    // Pinned bool_bits: ACharacter+0x321 mask4 = bWasJumping (DWORD+320 bit10).
    const bool bAlreadyJumping = CharacterOwner && CharacterOwner->bWasJumping;
    const float SavedJumpZVelocity = JumpZVelocity;
    uint32 PenaltyIndex = static_cast<uint32>(NativeJumpPenaltyIndex1e54);
    if (!bAlreadyJumping)
    {
        // Original component WorldPrivate+a8, with1440b3260 fallback when null.
        UWorld* World = GetWorld();
        AFortGameStateAthena* GameState = World ? Cast<AFortGameStateAthena>(World->GetGameState()) : nullptr;
        if ((GameState && !GameState->NativeEvaluateJumpPenaltyMutators1419aff90(CharacterOwner))
            || NativeJumpPenaltyRemaining1e58 <= 0.0f)
        {
            PenaltyIndex = 0;
        }
        else
        {
            // Preserve original unsigned min/increment including wrap; no invented index clamp.
            const uint32 Next = PenaltyIndex + 1;
            const uint32 Last = static_cast<uint32>(JumpPenalties.Num() - 1);
            PenaltyIndex = Next <= Last ? Next : Last;
        }
    }
    JumpZVelocity *= JumpPenalties[static_cast<int32>(PenaltyIndex)].JumpScalar;
    if (bSpecialJump)
    {
        //1440739b0 -> ProcessEvent(NAME_CanJumpInternal), NOT CanJump()/engine DoJump.
        bSuccess = CharacterOwner && NativeCanJumpInternalEvent1440739b0(CharacterOwner);
    }
    else
    {
        bSuccess = Super::DoJump(bReplayingMoves);
    }
    if (bSuccess && !bAlreadyJumping)
    {
        NativeJumpPenaltyIndex1e54 = static_cast<int32>(PenaltyIndex);
        NativeJumpPenaltyRemaining1e58 = JumpPenaltyResetTime;
        // Original assumes owner valid on this successful path; no fabricated null fallback.
        AFortPawn* Pawn = static_cast<AFortPawn*>(CharacterOwner);
        uint8 Style;
        float Speed = Pawn->NativeGetMovementSpeed14231ab10(*Pawn->NativeGetCurrentMovementStyle14231a980(&Style));
        if (IsExceedingMaxSpeed(Speed)) // original virtual+418; includes Velocity.Z in predicate.
        {
            Speed = FMath::Sqrt(Velocity.X * Velocity.X + Velocity.Y * Velocity.Y);
        }
        const float Cap = Speed * JumpPenalties[static_cast<int32>(PenaltyIndex)].MovementScalar;
        if (!(Cap >= 0.0001f)) // Unordered cap clears XY too.
        {
            Velocity.X = 0.0f;
            Velocity.Y = 0.0f;
        }
        else
        {
            const float XY2 = Velocity.X * Velocity.X + Velocity.Y * Velocity.Y;
            if (XY2 > Cap * Cap) // COMISS XY2,Cap2; JBE retains XY for unordered too.
            {
                // Original RSQRTSS estimate and TWO Newton steps, no generic sqrt substitution.
                const float HalfXY2 = XY2 * 0.5f;
                float R = _mm_cvtss_f32(_mm_rsqrt_ss(_mm_set_ss(XY2)));
                R = R + R * (0.5f - HalfXY2 * (R * R));
                R = R + R * (0.5f - HalfXY2 * (R * R));
                const float Scale = R * Cap;
                Velocity.X *= Scale;
                Velocity.Y *= Scale;
            }
        }
        // Velocity.Z is bitwise retained by original XY assignment.
    }
    JumpZVelocity = SavedJumpZVelocity;
}

if (CharacterOwner && CharacterOwner->GetLocalRole() == ROLE_Authority && bSuccess && !bReplayingMoves)
{
    if (AFortPlayerPawnAthena* Pawn = Cast<AFortPlayerPawnAthena>(CharacterOwner))
    {
        // Original virtual+9a8 target142316940; optional UPrimitiveComponent** argument null.
        AActor* FloorActor = Pawn->NativeGetFloorActor142316940(nullptr);
        if (ABuildingSMActor* Building = Cast<ABuildingSMActor>(FloorActor))
        {
            // Shared TWeakObjectPtr<AController> field+718; constructor default proven in FOLLOWUP.md.
            // Keep separate IsValid then Get; do not reset FallInstigator if invalid.
            if (Building->NativeFloorAttributionWeak718.IsValid())
            {
                Pawn->FallInstigator = Building->NativeFloorAttributionWeak718.Get();
            }
            Pawn->LastFloorBeforeFalling = Building;
        }
    }
}
NativeDoJumpFlag1dfd = PreviousFlag;
return bSuccess;
