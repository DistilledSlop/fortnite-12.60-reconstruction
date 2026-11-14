// CONFIDENCE: HIGH. Complete original body; class lifecycle acceptance remains separate.
// 0x14230bcb0, AFortPlayerPawn virtual +0x1328.
// Dependencies and initialization are specified in contracts.json.
const FFortNativeGameplayTagsRuntime& NativeTags = FFortNativeGameplayTagsRuntime::Get();
if (!NativeCanPerformAction14230c260(NativeTags.GetTagAtOriginalOffset(0x798))) return false;
if (CurrentSkyTube) return false;
UCharacterMovementComponent* Move = GetCharacterMovement();
// Original dereferences the movement component before its later null check.
if (Move->MovementMode == MOVE_Custom && Move->CustomMovementMode == 10) return false;
if (bLocalIsSkydiving || bIsParachuteOpen) return true;
if (!Move || Move->MovementMode != MOVE_Falling) return false;
if (!UFortMovementComp_CharacterAthena::NativeCanJumpInternalEvent1440739b0(this) &&
    NativeEnableParachuteEverywhere148157cf4() != 0) return true;
if (bIsDBNO) return false;
UWorld* World = GetWorld();
if (!World) return false;

// Original ANDPS clears the sign bit (including negative zero and NaNs).
const auto NativeAbsBits = [](float Value) {
    uint32 Bits;
    FMemory::Memcpy(&Bits, &Value, sizeof(Bits));
    Bits &= 0x7fffffffU;
    FMemory::Memcpy(&Value, &Bits, sizeof(Value));
    return Value;
};
bool bRedeploy = GliderRedeployAllowedRow.GetValueAtLevel(0.0f, nullptr) > 0.0f;
float Height = NativeAbsBits(GliderRedeployHeighLimitRow.GetValueAtLevel(0.0f, nullptr));
float Lateral = NativeAbsBits(GliderRedeployLateralVelocityMultiplierRow.GetValueAtLevel(0.0f, nullptr));
if (AFortGameStateAthena* GameState = Cast<AFortGameStateAthena>(World->GetGameState()))
{
    bRedeploy = GameState->NativeQueryGliderRedeploy14198b790(this);
    Height = GameState->DefaultRedeployGliderHeightLimit;
    Lateral = GameState->DefaultRedeployGliderLateralVelocityMult;
}
UAbilitySystemComponent* AbilitySystem = GetAbilitySystemComponent();
FGameplayTagContainer Tags;
Tags.AddTag(NativeTags.GetTagAtOriginalOffset(0x1848));
Tags.AddTag(NativeTags.GetTagAtOriginalOffset(0x7e0));
if (!bRedeploy && (!AbilitySystem || !AbilitySystem->HasAnyMatchingGameplayTags(Tags))) return false;
if (bLocalIsSkydiving) return false;
// The subsequent original tests of skydiving-from-bus and launch-pad both
// require bLocalIsSkydiving and thus are unreachable after the test above.
if (NativeGliderRedeployRequiresJump148157d08() > 0 &&
    !(NativeAbsBits(JumpLastActivatedTime - -1.0f) > 1.e-8f)) return false;
if (NativeGliderRedeployUseWindowOfTime148157d0c() > 0 &&
    World->GetTimeSeconds() - JumpLastActivatedTime >= NativeGliderRedeployWindowLength1478de4b0()) return false;
const float Now = World->GetTimeSeconds();
if (!(Now - NativeParachuteTraceTime2280 >= 0.1f)) return bNativeParachuteTraceResult10e8Bit7;
NativeParachuteTraceTime2280 = Now;
if (!(Height > 0.0f)) return false;
USceneComponent* InitialRoot = GetRootComponent();
const FVector Start = InitialRoot ? InitialRoot->GetComponentLocation() : FVector::ZeroVector;
const FVector Velocity = GetVelocity();
const FVector End(Start.X + Lateral * Velocity.X, Start.Y + Lateral * Velocity.Y, Start.Z + -Height);
UCapsuleComponent* Capsule = Cast<UCapsuleComponent>(GetRootComponent());
if (!Capsule) return false;
const FVector Scale = Capsule->GetComponentScale();
// MINSS(scale.Y, scale.X) selects the second operand on equality or unordered.
const float Radius = (Scale.Y < Scale.X ? Scale.Y : Scale.X) * Capsule->GetUnscaledCapsuleRadius();
const float HalfHeight = Capsule->GetUnscaledCapsuleHalfHeight();
FHitResult Hit;
const bool bHit = UKismetSystemLibrary::CapsuleTraceSingle(
    this, Start, End, Radius, HalfHeight,
    UEngineTypes::ConvertToTraceType(ECC_GameTraceChannel11), false, TArray<AActor*>(),
    EDrawDebugTrace::None, Hit, true, FLinearColor::Red, FLinearColor::Green, 5.0f);
bNativeParachuteTraceResult10e8Bit7 = !bHit || Hit.Distance >= Height;
return bNativeParachuteTraceResult10e8Bit7;
