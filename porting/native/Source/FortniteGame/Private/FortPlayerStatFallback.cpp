#include "FortPlayerStatFallback.h"
#include "FFortPlayerAttributeSets.h"
#include "FFortPlayerPawnStats.h"
#include "UFortHealthSet.h"
#include "UFortRegenHealthSet.h"
#include "UFortControlResistanceSet.h"
#include "UFortMovementSet.h"
#include "UFortAdvancedMovementSet.h"
#include "UFortPlayerAttrSet.h"
#include "AbilitySystemComponent.h"
#include "AFortPlayerState.h"
#include "AFortPlayerController.h"
#include "AFortGameState.h"
#include "AFortGameStateZone.h"
#include "FGameDifficultyInfo.h"
#include "EStatRecordingPeriod.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/Parse.h"
#include "UObject/UnrealType.h"

namespace FortPlayerStatFallback
{
namespace
{
void SetIfAbsent(UAttributeSet* Set, FProperty* Property, float Value,
                 const TSet<FProperty*>& Initialized)
{
    if (!Initialized.Contains(Property))
    {
        if (UAbilitySystemComponent* ASC = Set->GetOwningAbilitySystemComponent())
            ASC->SetNumericAttributeBase(FGameplayAttribute(Property), Value);
    }
}

}

void Apply(FFortPlayerAttributeSets& Sets, const FFortPlayerPawnStats& Stats,
           const FGameDifficultyInfo* Context, const TSet<FProperty*>& InitializedProperties)
{
#define WRITE(SET, CLASS, FIELD, VALUE) \
    SetIfAbsent((SET), FindFieldChecked<FProperty>(CLASS::StaticClass(), GET_MEMBER_NAME_CHECKED(CLASS, FIELD)), (VALUE), InitializedProperties)
    if (auto* S = Sets.HealthSet)
    {
        WRITE(S, UFortHealthSet, MaxHealth, Stats.MaximumHealth);
        WRITE(S, UFortHealthSet, Health, S->MaxHealth.GetCurrentValue());
        WRITE(S, UFortRegenHealthSet, HealthRegenRate, Stats.HealthRegenRate);
        WRITE(S, UFortRegenHealthSet, HealthRegenDelay, Stats.HealthRegenDelay);
        WRITE(S, UFortRegenHealthSet, HealthRegenThreshold, Stats.HealthRegenThreshold);
        WRITE(S, UFortHealthSet, Shield, Stats.MaxShield);
        WRITE(S, UFortHealthSet, CurrentShield, S->Shield.GetCurrentValue());
        WRITE(S, UFortRegenHealthSet, ShieldRegenRate, Stats.ShieldRegenRate);
        WRITE(S, UFortRegenHealthSet, ShieldRegenDelay, Stats.ShieldRegenDelay);
        WRITE(S, UFortRegenHealthSet, ShieldRegenThreshold, Stats.ShieldRegenThreshold);
    }
    if (auto* S = Sets.ControlResistanceSet)
    {
        WRITE(S, UFortControlResistanceSet, MaxControlResistance, Stats.MaxControlResistance);
        WRITE(S, UFortControlResistanceSet, ControlResistance, S->MaxControlResistance.GetCurrentValue());
        WRITE(S, UFortControlResistanceSet, ControlResistanceRegenRate, Stats.ControlResistanceRegenRate);
        WRITE(S, UFortControlResistanceSet, ControlResistanceRegenDelay, Stats.ControlResistanceRegenDelay);
        WRITE(S, UFortControlResistanceSet, ControlResistanceRegenThreshold, Stats.ControlResistanceRegenThreshold);
        WRITE(S, UFortControlResistanceSet, KnockbackMultiplier, Stats.KnockbackMultiplier);
        WRITE(S, UFortControlResistanceSet, KnockbackThreshold, Stats.KnockbackThreshold);
    }
    if (auto* S = Sets.MovementSet)
    {
        WRITE(S, UFortMovementSet, WalkSpeed, Stats.SpeedWalk);
        WRITE(S, UFortMovementSet, RunSpeed, Stats.SpeedRun);
        WRITE(S, UFortMovementSet, SprintSpeed, Stats.SpeedSprint);
        WRITE(S, UFortMovementSet, FlySpeed, Stats.SpeedFly);
        WRITE(S, UFortMovementSet, CrouchedRunSpeed, Stats.SpeedCrouchedRun);
        WRITE(S, UFortMovementSet, CrouchedSprintSpeed, Stats.SpeedCrouchedSprint);
        WRITE(S, UFortMovementSet, BackwardSpeedMultiplier, Stats.SpeedBackwardsMultiplier);
    }
    if (auto* S = Sets.AdvancedMovementSet)
    {
        WRITE(S, UFortAdvancedMovementSet, GroundFriction, Stats.GroundFriction);
        WRITE(S, UFortAdvancedMovementSet, BrakingDecelerationWalking, Stats.BrakingDecelerationWalking);
        WRITE(S, UFortAdvancedMovementSet, BrakingDecelerationFalling, Stats.BrakingDecelerationFalling);
        WRITE(S, UFortAdvancedMovementSet, BrakingDecelerationFlying, Stats.BrakingDecelerationFlying);
        WRITE(S, UFortAdvancedMovementSet, MaxAcceleration, Stats.MaxAcceleration);
        WRITE(S, UFortAdvancedMovementSet, MaxAccelerationFlying, Stats.MaxAccelerationFlying);
        WRITE(S, UFortAdvancedMovementSet, BrakingFrictionFactor, Stats.BrakingFrictionFactor);
        WRITE(S, UFortAdvancedMovementSet, JumpZVelocity, Stats.JumpZVelocity);
        WRITE(S, UFortAdvancedMovementSet, MinAnalogWalkSpeed, Stats.MinAnalogWalkSpeed);
    }
    if (auto* S = Sets.PlayerAttrSet)
    {
        WRITE(S, UFortPlayerAttrSet, MaxStamina, Stats.MaxStamina);
        WRITE(S, UFortPlayerAttrSet, Stamina, S->MaxStamina.GetCurrentValue());
        WRITE(S, UFortPlayerAttrSet, StaminaRegenRate, Stats.StaminaRegenRate);
        WRITE(S, UFortPlayerAttrSet, StaminaRegenDelay, Stats.StaminaRegenDelay);
        WRITE(S, UFortPlayerAttrSet, SprintCost, Stats.SprintingStaminaExpenditureRate);
        WRITE(S, UFortPlayerAttrSet, CraftingTimeMultiplier, 1.0f);
        if (Context && S->GetOuter() && S->ExtraLives.GetCurrentValue() < 0.0f)
        {
            int32 Lives = Context->DefaultPlayerLives;
            if (AFortPlayerState* Owner = Cast<AFortPlayerState>(S->GetOuter()))
                if (AFortPlayerController* Controller = Cast<AFortPlayerController>(Owner->GetOwner()))
                    Lives -= Controller->GetNativeStatValue(FName(TEXT("SelfRevives")), EStatRecordingPeriod::Map);
            WRITE(S, UFortPlayerAttrSet, ExtraLives, float(Lives));
        }
        if (S->SprintCost.GetCurrentValue() > 0.0f)
            WRITE(S, UFortPlayerAttrSet, SprintCost, -S->SprintCost.GetCurrentValue());
    }
#undef WRITE
}
}

void FortPlayerStatFallback::ApplyForPlayerState(AFortPlayerState& PlayerState,
    const FFortPlayerPawnStats& Stats, const TSet<FProperty*>& InitializedProperties)
{
    const FGameDifficultyInfo* Context = nullptr;
    if (UWorld* World = PlayerState.GetWorld())
    {
        if (AFortGameStateZone* Zone = World->GetGameState<AFortGameStateZone>())
        {
            Context = Zone->ZoneDifficultyInfoRow.GetRow<FGameDifficultyInfo>(
                TEXT("AFortGameStateZone::GetZoneDifficultyInfo"));
            if (!Context)
            {
                if (AFortGameState* State = Cast<AFortGameState>(World->GetGameState()))
                    if (State->WorldManager)
                    {
                        static const bool bNoMcp = FParse::Param(FCommandLine::Get(), TEXT("nomcp"));
                        (void)bNoMcp;
                    }
            }
        }
    }
    Apply(PlayerState.AttributeSets, Stats, Context, InitializedProperties);
}
