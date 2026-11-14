// Fall damage re-creation.
//   data   /Game/Athena/Balance/DataTables/AthenaFallingDamage row `Player`: fall speed -> damage fraction
//          (D4-game-data.md: 1300:0.1, 1536:0.2, 1920:0.5, 2304:1.0). AFortPawn::GetMaxDistanceWithoutFallDamage
//          (0x142321b30 -> 0x142321b50 "FallingDamageTable Lookup") scans the same row for its first non-zero key.
//   binary 0x14231b6e0 evaluates the row curve for a given value and returns the fraction.
//   stand-in: the original applies the fraction through GET_FallingDamage / an execution calculation that has no ported
//          body. The damage here is fraction * MaxHealth, ignoring shields (D6 3: fall damage ignores shields), tagged with
//          the native FallDamage death-cause tag (original tag table offset 0x358, AFortPlayerStateAthena::ToDeathCause 0x141a5f1c0).
//   stand-in: the landing speed is sampled from the pawn's velocity on the last tick it was MOVE_Falling, not from the
//          original Landed hit. bIgnoreNextFallingDamage is honoured and cleared. Custom movement modes (skydive/parachute/
//          glider) never accumulate speed, so gliding landings do no damage.
#include "FortAthenaFallDamage.h"
#include "FortAthenaHealth.h"
#include "FortNativeGameplayTagsRuntime.h"
#include "AFortPlayerPawnAthena.h"
#include "Engine/CurveTable.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "UObject/UObjectGlobals.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortFallDamage, Log, All);

namespace
{
	struct FFallState
	{
		float PeakSpeed = 0.0f;     // largest downward speed seen while MOVE_Falling
		float StartZ = 0.0f;
		float PeakZ = 0.0f;
		bool bWasFalling = false;
	};
	TMap<TWeakObjectPtr<AFortPlayerPawnAthena>, FFallState> GStates;

	const FRealCurve* GetCurve()
	{
		static TWeakObjectPtr<UCurveTable> Table;
		static bool bTried = false;
		if (!Table.IsValid() && !bTried)
		{
			bTried = true;
			Table = LoadObject<UCurveTable>(nullptr, TEXT("/Game/Athena/Balance/DataTables/AthenaFallingDamage.AthenaFallingDamage"));
			UE_LOG(LogFortFallDamage, Display, TEXT("FallDamage: table %s"), Table.IsValid() ? TEXT("loaded") : TEXT("NOT FOUND (no fall damage)"));
		}
		return Table.IsValid() ? Table->FindCurve(FName(TEXT("Player")), TEXT("FallDamage"), false) : nullptr;
	}
}

void FortAthenaFallDamage::Tick(UWorld* World)
{
	if (!World || !FortAthenaHealth::IsEnabled())
		return;
	const FRealCurve* Curve = GetCurve();
	if (!Curve)
		return;

	for (TActorIterator<AFortPlayerPawnAthena> It(World); It; ++It)
	{
		AFortPlayerPawnAthena* Pawn = *It;
		UCharacterMovementComponent* Movement = Pawn ? Pawn->GetCharacterMovement() : nullptr;
		if (!Movement || Pawn->bIsDying)
			continue;
		FFallState& State = GStates.FindOrAdd(Pawn);
		const bool bFalling = Movement->MovementMode == MOVE_Falling && !Pawn->bIsSkydiving && !Pawn->bIsParachuteOpen;
		if (bFalling)
		{
			if (!State.bWasFalling)
			{
				State = FFallState();
				State.bWasFalling = true;
				State.StartZ = State.PeakZ = Pawn->GetActorLocation().Z;
			}
			State.PeakSpeed = FMath::Max(State.PeakSpeed, -Movement->Velocity.Z);
			State.PeakZ = FMath::Max(State.PeakZ, Pawn->GetActorLocation().Z);
			continue;
		}
		if (!State.bWasFalling)
			continue;
		// Left ordinary falling this tick.
		const bool bLanded = Movement->IsMovingOnGround() || Movement->MovementMode == MOVE_Swimming;
		const float Speed = State.PeakSpeed;
		const float Drop = State.PeakZ - Pawn->GetActorLocation().Z;
		State = FFallState();
		if (!bLanded)
			continue;  // entered a custom mode (glider, zipline, ...): no damage
		if (Pawn->bIgnoreNextFallingDamage)
		{
			Pawn->bIgnoreNextFallingDamage = 0;
			UE_LOG(LogFortFallDamage, Display, TEXT("FallDamage: %s landed at %.0f cm/s (drop %.0f) - ignored (bIgnoreNextFallingDamage)"), *Pawn->GetName(), Speed, Drop);
			continue;
		}
		if (Movement->MovementMode == MOVE_Swimming)
			continue;
		const float Fraction = FMath::Clamp(Curve->Eval(Speed, 0.0f), 0.0f, 1.0f);
		if (Fraction <= 0.0f)
			continue;
		const float Damage = Fraction * Pawn->GetMaxHealth();
		FGameplayTagContainer Tags;
		const FGameplayTag FallTag = FFortNativeGameplayTagsRuntime::Get().GetTagAtOriginalOffset(0x358);
		if (FallTag.IsValid())
			Tags.AddTag(FallTag);
		UE_LOG(LogFortFallDamage, Display, TEXT("FallDamage: %s landed at %.0f cm/s (drop %.0f): fraction %.2f -> %.1f damage"), *Pawn->GetName(), Speed, Drop, Fraction, Damage);
		FortAthenaHealth::ApplyDamage(Pawn, Damage, Tags, nullptr, nullptr, true);
	}
	for (auto It = GStates.CreateIterator(); It; ++It)
		if (!It.Key().IsValid())
			It.RemoveCurrent();
}
