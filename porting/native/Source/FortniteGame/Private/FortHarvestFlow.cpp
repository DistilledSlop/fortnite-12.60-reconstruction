// Server-side pickaxe harvest flow for the original client.
//
// The original melee ability (GAB_Melee_Generic_EventGraph) waits for the client's target data through the native task
// UFortAbilityTask_PlayMontageWaitTarget and applies GE_Melee_GenericDamage through ApplyGameplayEffectContainer. Those natives are
// not declared in our generated module, so the blueprint cannot run them. This file receives the same target data (engine hook
// UAbilitySystemComponent::FortServerTargetDataReceived, patch ue425-fortnite-asc-server-target-data-hook) and drives the original
// damage/resource logic ported in FortHarvestDamage / FortHarvestBuilding.
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbilityTargetTypes.h"
#include "Abilities/GameplayAbility.h"
#include "Engine/EngineBaseTypes.h"
#include "Engine/World.h"
#include "Misc/CommandLine.h"
#include "Misc/CoreDelegates.h"
#include "Misc/Parse.h"
#include "ABuildingSMActor.h"
#include "AFortPawn.h"
#include "AFortPlayerController.h"
#include "AFortPlayerState.h"
#include "AFortWeapon.h"
#include "UFortWeaponItemDefinition.h"
#include "GameFramework/Actor.h"
#include "FortHarvestBuilding.h"
#include "FortHarvestDamage.h"
#include "FortAthenaHealth.h"
#include "FFortRangedWeaponStats.h"
#include "Engine/DataTable.h"
#include "GameplayTagContainer.h"

DEFINE_LOG_CATEGORY_STATIC(LogFortHarvest, Log, All);

namespace FortHarvest
{
    namespace
    {
        FDelegateHandle GTargetDataHandle;

        bool IsMeleeAbility(const UGameplayAbility* Ability)
        {
            if (!Ability)
                return false;
            const FString Name = Ability->GetClass()->GetName();
            return Name.Contains(TEXT("Melee"));
        }

        TMap<TWeakObjectPtr<const AController>, double>& LastSwingTime()
        {
            static TMap<TWeakObjectPtr<const AController>, double> Times;
            return Times;
        }

        EHitSurface SurfaceFromResource(const ABuildingSMActor* Building)
        {
            // stand-in: the original reads the physical surface of the hit material (SurfaceDamageCategory columns). The server stage
            // omits materials, so use the building's resource type, which is the same wood/stone/metal classification.
            switch (Building->ResourceType.GetValue())
            {
            case EFortResourceType::Wood: return EHitSurface::Wood;
            case EFortResourceType::Stone: return EHitSurface::Stone;
            case EFortResourceType::Metal: return EHitSurface::Metal;
            default: return EHitSurface::Default;
            }
        }

        bool IsWeakSpotHit(const AFortPlayerController* PC, const ABuildingSMActor* Building, const FHitResult& Hit)
        {
            // stand-in: the client decides the weak-spot hit locally. The server keeps the spot in PC->ActiveWeakSpots (original
            // 0x1423b5c20); a hit within this radius of the stored position counts. The wire format of the client's report is not yet known.
            constexpr float WeakSpotRadius = 80.0f;
            const FVector Impact(Hit.ImpactPoint);
            for (const FBuildingWeakSpotData& Spot : PC->ActiveWeakSpots)
            {
                if (Spot.ParentBuilding.Get() == Building && FVector::Dist(FVector(Spot.Position), Impact) <= WeakSpotRadius)
                    return true;
            }
            return false;
        }


        // ---- ranged weapons -------------------------------------------------------------------------------------------------
        // The original ranged ability (FortGameplayAbility_RangedWeapon, native) is not ported, so the server never runs the fire,
        // trace and damage logic. The client still predicts the shot and reports its hit through the same target-data RPC the melee
        // ability uses, so the hit arrives here. Damage is computed on the server from the weapon's stat row (never from the
        // client), the hit is range-checked against the row, and ammo is consumed from the replicated weapon.
        // stand-in: the original per-bullet validation, spread/recoil, falloff curve rounding and effect-container path are not
        // reproduced; falloff is a linear blend between the row's range bands.
        TMap<TWeakObjectPtr<const AController>, double>& LastShotTime()
        {
            static TMap<TWeakObjectPtr<const AController>, double> Times;
            return Times;
        }

        float RangedFalloffDamage(const FFortBaseWeaponStats& Row, float Distance)
        {
            const float Bands[4] = { Row.RngPB, Row.RngMid, Row.RngLong, Row.RngMax };
            const float Dmg[4] = { Row.DmgPB, Row.DmgMid, Row.DmgLong, Row.DmgMaxRange };
            if (Distance <= Bands[0] || Bands[1] <= Bands[0])
                return Dmg[0];
            for (int32 I = 0; I < 3; ++I)
            {
                if (Distance <= Bands[I + 1] && Bands[I + 1] > Bands[I])
                    return FMath::Lerp(Dmg[I], Dmg[I + 1], (Distance - Bands[I]) / (Bands[I + 1] - Bands[I]));
            }
            return Dmg[3];
        }

        void OnRangedTargetData(UAbilitySystemComponent* ASC, const UGameplayAbility* Ability, const FGameplayAbilityTargetDataHandle& TargetData)
        {
            AFortPawn* Pawn = Cast<AFortPawn>(ASC->GetAvatarActor());
            AFortPlayerController* PC = Pawn ? Cast<AFortPlayerController>(Pawn->GetController()) : nullptr;
            AFortWeapon* Weapon = Pawn ? Pawn->CurrentWeapon : nullptr;
            const UFortWeaponItemDefinition* WeaponDef = Weapon ? Weapon->WeaponData : nullptr;
            const FDataTableRowHandle StatHandle = WeaponDef ? WeaponDef->GetWeaponStatHandle() : FDataTableRowHandle();
            const FFortBaseWeaponStats* Row = StatHandle.DataTable ? StatHandle.GetRow<FFortBaseWeaponStats>(TEXT("RangedTargetData")) : nullptr;
            static int32 Logged = 0;
            if (Logged++ < 200)
            {
                UE_LOG(LogFortHarvest, Display, TEXT("Combat: RangedTargetData ability=%s pawn=%s weapon=%s def=%s row=%s rowFound=%d ammo=%d entries=%d"),
                    *GetNameSafe(Ability), *GetNameSafe(Pawn), *GetNameSafe(Weapon), *GetNameSafe(WeaponDef), *StatHandle.RowName.ToString(),
                    Row != nullptr, Weapon ? Weapon->AmmoCount : -1, TargetData.Num());
            }
            if (!PC || !Weapon || !Row)
                return;
            UWorld* World = Pawn->GetWorld();
            const double Now = World ? World->GetTimeSeconds() : 0.0;
            double& Last = LastShotTime().FindOrAdd(PC);
            if (Now - Last < 0.04)
                return;
            Last = Now;

            // Ammo: one bullet batch per accepted report (CartridgePerFire hits can share one report).
            const int32 AmmoCost = FMath::Max(Row->AmmoCostPerFire, 1);
            // Ammo is deliberately NOT changed on the server. The client predicts consumption and reload locally; the server has no
            // reload implementation, so a server-side decrement would be replicated back over the client's reloaded value (AmmoCount
            // replicates on change) and make reloads appear to fail. The count is logged so a later reload port can verify it.
            UE_LOG(LogFortHarvest, Verbose, TEXT("Combat: RangedShot weapon=%s serverAmmo=%d cost=%d (not modified)"), *GetNameSafe(Weapon), Weapon->AmmoCount, AmmoCost);

            const uint8 ShooterTeam = [&]() { AFortPlayerState* S = Cast<AFortPlayerState>(PC->PlayerState); return S ? S->NativeGetTeam163() : uint8(255); }();
            for (int32 Index = 0; Index < TargetData.Num(); ++Index)
            {
                const FGameplayAbilityTargetData* Data = TargetData.Get(Index);
                const FHitResult* Hit = Data ? Data->GetHitResult() : nullptr;
                if (!Hit)
                    continue;
                AActor* Target = Hit->GetActor();
                const FVector Impact(Hit->ImpactPoint);
                const float Distance = FVector::Dist(Pawn->GetActorLocation(), Impact);
                const float MaxRange = FMath::Max(Row->RngMax, Row->RngLong) * 1.25f + 500.0f;
                if (!Target || Distance > MaxRange)
                {
                    UE_LOG(LogFortHarvest, Display, TEXT("Combat: RangedHit [%d] ignored target=%s distance=%.0f maxRange=%.0f"), Index, *GetNameSafe(Target), Distance, MaxRange);
                    continue;
                }
                if (ABuildingSMActor* Building = Cast<ABuildingSMActor>(Target))
                {
                    const float EnvDamage = Row->EnvDmgPB > 0.0f ? Row->EnvDmgPB : Row->DmgPB * 0.5f;
                    const bool bApplied = FortHarvest::ApplyBuildingDamage(Building, EnvDamage, PC, Pawn, *Hit, false);
                    UE_LOG(LogFortHarvest, Display, TEXT("Combat: RangedHit [%d] building=%s damage=%.1f applied=%d"), Index, *GetNameSafe(Building), EnvDamage, bApplied);
                    continue;
                }
                AFortPawn* Victim = Cast<AFortPawn>(Target);
                if (!Victim || Victim == Pawn)
                {
                    UE_LOG(LogFortHarvest, Display, TEXT("Combat: RangedHit [%d] target=%s is not damageable here"), Index, *GetNameSafe(Target));
                    continue;
                }
                AFortPlayerState* VictimState = Cast<AFortPlayerState>(Victim->GetPlayerState());
                if (VictimState && ShooterTeam != 255 && VictimState->NativeGetTeam163() == ShooterTeam)
                {
                    UE_LOG(LogFortHarvest, Display, TEXT("Combat: RangedHit [%d] ignored, same team"), Index);
                    continue;
                }
                const bool bHeadshot = Hit->BoneName.ToString().Contains(TEXT("head"), ESearchCase::IgnoreCase);
                float Damage = RangedFalloffDamage(*Row, Distance);
                if (bHeadshot && Row->DamageZone_Critical > 0.0f)
                    Damage *= Row->DamageZone_Critical;
                const float Before = Victim->GetHealth() + Victim->GetShield();
                FortAthenaHealth::ApplyDamage(Victim, Damage, FGameplayTagContainer(), PC, Weapon, false);
                UE_LOG(LogFortHarvest, Display, TEXT("Combat: RangedHit [%d] victim=%s damage=%.1f headshot=%d distance=%.0f hp+shield %.1f -> %.1f dying=%d"),
                    Index, *GetNameSafe(Victim), Damage, bHeadshot, Distance, Before, Victim->GetHealth() + Victim->GetShield(), Victim->bIsDying ? 1 : 0);
            }
        }

        void OnTargetData(UAbilitySystemComponent* ASC, FGameplayAbilitySpecHandle SpecHandle,
                          const FGameplayAbilityTargetDataHandle& TargetData, FGameplayTag ApplicationTag)
        {
            const FGameplayAbilitySpec* Spec = ASC ? ASC->FindAbilitySpecFromHandle(SpecHandle) : nullptr;
            const UGameplayAbility* Ability = Spec ? Spec->Ability : nullptr;
            if (!Ability)
                return;
            if (!IsMeleeAbility(Ability))
            {
                if (Ability->GetClass()->GetName().Contains(TEXT("Ranged")))
                    OnRangedTargetData(ASC, Ability, TargetData);
                return;
            }
            AFortPawn* Pawn = Cast<AFortPawn>(ASC->GetAvatarActor());
            AFortPlayerController* PC = Pawn ? Cast<AFortPlayerController>(Pawn->GetController()) : nullptr;
            const AFortWeapon* Weapon = Pawn ? Pawn->CurrentWeapon : nullptr;
            const UFortWeaponItemDefinition* WeaponDef = Weapon ? Weapon->WeaponData : nullptr;
            const FName WeaponRow = WeaponDef ? WeaponDef->GetWeaponStatHandle().RowName : NAME_None;
            FMeleeStats Stats;
            UE_LOG(LogFortHarvest, Display, TEXT("Harvest: target data ability=%s entries=%d tag=%s pawn=%s weapon=%s row=%s"),
                *GetNameSafe(Ability), TargetData.Num(), *ApplicationTag.ToString(), *GetNameSafe(Pawn), *GetNameSafe(WeaponDef), *WeaponRow.ToString());
            if (!PC || !FindMeleeStats(WeaponRow, Stats))
            {
                UE_LOG(LogFortHarvest, Display, TEXT("Harvest: ignored (controller=%d, melee stats row known=%d)"), PC != nullptr, FindMeleeStats(WeaponRow, Stats));
                return;
            }
            UWorld* World = Pawn->GetWorld();
            const double Now = World ? World->GetTimeSeconds() : 0.0;
            double& Last = LastSwingTime().FindOrAdd(PC);
            if (Now - Last < 0.25 * 0.5667)
            {
                UE_LOG(LogFortHarvest, Display, TEXT("Harvest: ignored, swing too soon (%.3f s)"), Now - Last);
                return;
            }
            Last = Now;

            for (int32 Index = 0; Index < TargetData.Num(); ++Index)
            {
                const FGameplayAbilityTargetData* Data = TargetData.Get(Index);
                const FHitResult* Hit = Data ? Data->GetHitResult() : nullptr;
                if (!Hit)
                {
                    UE_LOG(LogFortHarvest, Display, TEXT("Harvest:  [%d] no hit result (type=%s)"), Index, Data ? *GetNameSafe(Data->GetScriptStruct()) : TEXT("null"));
                    continue;
                }
                AActor* Actor = Hit->GetActor();
                ABuildingSMActor* Building = Cast<ABuildingSMActor>(Actor);
                UE_LOG(LogFortHarvest, Display, TEXT("Harvest:  [%d] hit actor=%s class=%s component=%s impact=%s resource=%d"),
                    Index, *GetNameSafe(Actor), *GetNameSafe(Actor ? Actor->GetClass() : nullptr), *GetNameSafe(Hit->GetComponent()),
                    *Hit->ImpactPoint.ToString(), Building ? static_cast<int32>(Building->ResourceType.GetValue()) : -1);
                if (!Building)
                    continue;

                // Range check from the cooked melee row (RangeVSBuildings2D / RangeVSBuildingsZ).
                const FVector PawnLoc = Pawn->GetActorLocation();
                const FVector Impact(Hit->ImpactPoint);
                const float Dist2D = FVector::Dist2D(PawnLoc, Impact);
                const float DistZ = FMath::Abs(PawnLoc.Z - Impact.Z);
                if (Dist2D > Stats.RangeVsBuildings2D * 1.5f || DistZ > Stats.RangeVsBuildingsZ * 1.5f)
                {
                    UE_LOG(LogFortHarvest, Warning, TEXT("Harvest:  [%d] rejected, out of range (2D %.0f of %.0f, Z %.0f of %.0f)"),
                        Index, Dist2D, Stats.RangeVsBuildings2D, DistZ, Stats.RangeVsBuildingsZ);
                    continue;
                }

                // Team relation: level props are neutral; player-built pieces are friendly to their team.
                ETeamRelation Relation = ETeamRelation::Neutral;
                if (Building->bPlayerPlaced)
                {
                    AFortPlayerState* State = Cast<AFortPlayerState>(PC->PlayerState);
                    const uint8 PlayerTeam = State ? State->NativeGetTeam163() : 255;
                    Relation = (Building->TeamIndex == PlayerTeam) ? ETeamRelation::Friendly : ETeamRelation::Hostile;
                }
                const bool bWeakSpot = IsWeakSpotHit(PC, Building, *Hit);
                const EHitSurface Surface = bWeakSpot ? EHitSurface::WeakSpot : SurfaceFromResource(Building);
                float Damage = 0.0f;
                ComputeMeleeEnvironmentDamage(WeaponRow, Surface, Relation, Damage);
                const bool bApplied = FortHarvest::ApplyBuildingDamage(Building, Damage, PC, Pawn, *Hit, bWeakSpot);
                UE_LOG(LogFortHarvest, Display, TEXT("Harvest:  [%d] building=%s damage=%.1f weakSpot=%d relation=%d surface=%d health=%.1f/%.1f applied=%d"),
                    Index, *GetNameSafe(Building), Damage, bWeakSpot, static_cast<int32>(Relation), static_cast<int32>(Surface),
                    FortHarvest::GetBuildingHealth(Building), FortHarvest::GetBuildingMaxHealth(Building), bApplied);
            }
        }

        void Register()
        {
            if (!FParse::Param(FCommandLine::Get(), TEXT("FortniteOriginalClient")) || GTargetDataHandle.IsValid())
                return;
            // The generated ABuildingSMActor constructor does not set the original native defaults, and cooked blueprints only
            // serialize values that differ from them. Without this every harvestable prop reads bAllowResourceDrop=0 and
            // MaxResourcesToSpawn=0 (no materials ever). Original CDO: drops allowed, MaxResourcesToSpawn -1 (uncapped).
            // Must run before any blueprint subclass loads so their CDOs inherit it.
            if (ABuildingSMActor* Default = GetMutableDefault<ABuildingSMActor>())
            {
                Default->bAllowResourceDrop = true;
                Default->MaxResourcesToSpawn = -1;
            }
            GTargetDataHandle = UAbilitySystemComponent::FortServerTargetDataReceived.AddStatic(&OnTargetData);
            UE_LOG(LogFortHarvest, Display, TEXT("Harvest: listening for melee target data"));
        }

        FDelayedAutoRegisterHelper GRegister(EDelayedRegisterRunPhase::EndOfEngineInit, []() { Register(); });
    }
}
