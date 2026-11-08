// Behaviour snippet: AFortProjectileBase::GetOwnerWeapon (body only; spliced into the generated member function).
// Original body address: 0x1426cefa0 (thunk 0x142aa5e70)
// Offset evidence: this+0x788 = TWeakObjectPtr<UObject> cache (no reflected member; helpers 0x142ee7880 IsValid, 0x142ee5dd0 Get, 0x142ee3b30 operator=).
// AActor::Owner at +0xe0 is read through GetOwner() (private in UE 4.25). AController +0x248 = Pawn (reflected). AFortPawn +0x588 = CurrentWeapon (reflected).
// Callee evidence: 0x142b43730 AFortWeapon::StaticClass, 0x1446f1730 AController::StaticClass, 0x142a75f10 AFortPlayerPawn::StaticClass, 0x142a85850 AFortPlayerState::StaticClass, 0x1419b6d20 AFortPlayerState::GetCurrentPawn.
// CONFIDENCE: MEDIUM
static_assert(sizeof(AController) > 0, "AController must be complete");
static_assert(sizeof(AFortPlayerState) > 0, "AFortPlayerState must be complete");
TWeakObjectPtr<UObject>* WeakOwnerWeapon = reinterpret_cast<TWeakObjectPtr<UObject>*>(reinterpret_cast<uint8*>(this) + 0x788);
if (WeakOwnerWeapon->IsValid())
{
    return reinterpret_cast<AFortWeapon*>(WeakOwnerWeapon->Get());
}

AActor* Owner = GetOwner();
if (Owner != nullptr && Owner->IsA(AFortWeapon::StaticClass()))
{
    return reinterpret_cast<AFortWeapon*>(Owner);
}

AFortPlayerPawn* SourcePawn = nullptr;
if (Owner != nullptr)
{
    if (Owner->IsA(AController::StaticClass()))
    {
        APawn* ControllerPawn = static_cast<AController*>(Owner)->GetPawn();
        if (ControllerPawn != nullptr && ControllerPawn->IsA(AFortPlayerPawn::StaticClass()))
        {
            SourcePawn = reinterpret_cast<AFortPlayerPawn*>(ControllerPawn);
        }
    }
    if (SourcePawn == nullptr && Owner->IsA(AFortPlayerPawn::StaticClass()))
    {
        SourcePawn = reinterpret_cast<AFortPlayerPawn*>(Owner);
    }
    if (SourcePawn == nullptr && Owner->IsA(AFortPlayerState::StaticClass()))
    {
        SourcePawn = static_cast<AFortPlayerState*>(Owner)->GetCurrentPawn();
    }
}

if (SourcePawn != nullptr)
{
    AFortWeapon* Weapon = SourcePawn->CurrentWeapon;
    *WeakOwnerWeapon = Weapon;
    return Weapon;
}
return nullptr;
