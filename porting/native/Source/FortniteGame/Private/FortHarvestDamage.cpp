// Melee damage against buildings and props.
//
// Source data (read from the cooked game files, /Game/Balance/DataTables and /Game/Items/Datatables):
//   AthenaMeleeWeapons rows (RowStruct FortMeleeWeaponStats): EnvDmgPB..EnvDmgMaxRange are equal per row, EnvDmgScale 0 (no level scaling),
//     SurfaceRatioRowName AthenaMeleeHarvest for every row.
//   SurfaceDamageCategory row AthenaMeleeHarvest: Default->Default, Wood/Stone/Metal->AthenaHarvestBias, WeakSpot and WeakSpot_*->AthenaWeakSpot.
//   SurfaceDamageTeams rows Default (F0 N1 H1), AthenaHarvestBias (F1.5 N1 H1.5), AthenaWeakSpot (F3 N2 H3).
// The tables are embedded because their row structs (FortSurfaceDamageRatioByAffiliationStats, FortMeleeWeaponStats) are not declared in the
// generated module, so the DataTables do not load on the server. They must be regenerated if the generator learns those structs.
//
// Cross-check (community, webresearch/FINDINGS.md): pickaxe 50/100 against neutral props and 75/150 against player-built pieces
// (friendly) is exactly EnvDmg 50 * (1.0 | 2.0) and 50 * (1.5 | 3.0) from the rows above.
// Not read from the original binary: the full UFortDamageFormulaExecutionCalculation::Execute (0x141fe2cc0) was only skimmed
// (crit rating, armor, shield, distance falloff, level scaling terms); none of them apply to these rows (zero scales, equal bands).
#include "FortHarvestDamage.h"

namespace FortHarvest
{
	namespace
	{
		struct FMeleeRow { const TCHAR* Name; float Env; float Player; float R2D; float RZ; float RWeak; const TCHAR* Surface; };
		// AthenaMeleeWeapons (4 rows).
		const FMeleeRow GMeleeRows[] = {
			{ TEXT("Harvest_Pickaxe_Athena_C_T01"), 50.0f,    20.0f, 288.0f, 384.0f, 400.0f, TEXT("AthenaMeleeHarvest") },
			{ TEXT("Edged_Sword_Athena_C_T01"),     10050.0f, 75.0f, 288.0f, 384.0f, 400.0f, TEXT("AthenaMeleeHarvest") },
			{ TEXT("Galileo_Lobster_C_T01"),        200.0f,   50.0f, 288.0f, 384.0f, 400.0f, TEXT("AthenaMeleeHarvest") },
			{ TEXT("AdvancedMelee_ProtoAttack"),    200.0f,   50.0f, 288.0f, 384.0f, 400.0f, TEXT("AthenaMeleeHarvest") },
		};

		struct FTeamsRow { const TCHAR* Name; float Friendly; float Neutral; float Hostile; };
		// SurfaceDamageTeams (only the rows referenced by AthenaMeleeHarvest).
		const FTeamsRow GTeamsRows[] = {
			{ TEXT("Default"),           0.0f, 1.0f, 1.0f },
			{ TEXT("AthenaHarvestBias"), 1.5f, 1.0f, 1.5f },
			{ TEXT("AthenaWeakSpot"),    3.0f, 2.0f, 3.0f },
		};

		const TCHAR* CategoryTeamsRow(const FName SurfaceRatioRow, const EHitSurface Surface)
		{
			// SurfaceDamageCategory[AthenaMeleeHarvest]
			if (SurfaceRatioRow == FName(TEXT("AthenaMeleeHarvest")))
			{
				switch (Surface)
				{
				case EHitSurface::Wood:
				case EHitSurface::Stone:
				case EHitSurface::Metal: return TEXT("AthenaHarvestBias");
				case EHitSurface::WeakSpot: return TEXT("AthenaWeakSpot");
				default: return TEXT("Default");
				}
			}
			return nullptr;
		}
	}

	bool FindMeleeStats(const FName RowName, FMeleeStats& OutStats)
	{
		for (const FMeleeRow& Row : GMeleeRows)
		{
			if (RowName == FName(Row.Name))
			{
				OutStats.EnvDamage = Row.Env;
				OutStats.PlayerDamage = Row.Player;
				OutStats.RangeVsBuildings2D = Row.R2D;
				OutStats.RangeVsBuildingsZ = Row.RZ;
				OutStats.RangeVsWeakSpots = Row.RWeak;
				OutStats.SurfaceRatioRow = FName(Row.Surface);
				return true;
			}
		}
		return false;
	}

	float GetSurfaceRatio(const FName SurfaceRatioRow, const EHitSurface Surface, const ETeamRelation Relation)
	{
		const TCHAR* TeamsRow = CategoryTeamsRow(SurfaceRatioRow, Surface);
		if (!TeamsRow)
			return 0.0f;
		for (const FTeamsRow& Row : GTeamsRows)
		{
			if (FName(Row.Name) == FName(TeamsRow))
			{
				switch (Relation)
				{
				case ETeamRelation::Friendly: return Row.Friendly;
				case ETeamRelation::Neutral: return Row.Neutral;
				default: return Row.Hostile;
				}
			}
		}
		return 0.0f;
	}

	bool ComputeMeleeEnvironmentDamage(const FName WeaponRow, const EHitSurface Surface, const ETeamRelation Relation, float& OutDamage)
	{
		FMeleeStats Stats;
		if (!FindMeleeStats(WeaponRow, Stats))
			return false;
		OutDamage = Stats.EnvDamage * GetSurfaceRatio(Stats.SurfaceRatioRow, Surface, Relation);
		return true;
	}
}
