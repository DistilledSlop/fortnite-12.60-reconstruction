#pragma once

#include "CoreMinimal.h"

namespace FortHarvest
{
	enum class ETeamRelation : uint8 { Friendly, Neutral, Hostile };

	/** Surface classes of a hit, as used by the original SurfaceDamageCategory table columns. */
	enum class EHitSurface : uint8 { Default, Wood, Stone, Metal, WeakSpot };

	/** Per-weapon melee numbers read from the cooked table AthenaMeleeWeapons (row struct FortMeleeWeaponStats). */
	struct FMeleeStats
	{
		float EnvDamage = 0.0f;           // EnvDmgPB (all four range bands are equal in these rows)
		float PlayerDamage = 0.0f;        // DmgPB
		float RangeVsBuildings2D = 0.0f;  // RangeVSBuildings2D
		float RangeVsBuildingsZ = 0.0f;   // RangeVSBuildingsZ
		float RangeVsWeakSpots = 0.0f;    // RangeVSWeakSpots
		FName SurfaceRatioRow;            // SurfaceRatioRowName
	};

	/** Looks up an AthenaMeleeWeapons row by name (for example Harvest_Pickaxe_Athena_C_T01). */
	bool FindMeleeStats(FName RowName, FMeleeStats& OutStats);

	/** Surface damage ratio: SurfaceDamageCategory[SurfaceRatioRow][surface] -> SurfaceDamageTeams[row][relation]. */
	float GetSurfaceRatio(FName SurfaceRatioRow, EHitSurface Surface, ETeamRelation Relation);

	/**
	 * Damage a melee hit does to a building/prop: EnvDmg * surface/team ratio. A weak-spot hit uses the WeakSpot surface.
	 * Returns false when the weapon row is unknown.
	 */
	bool ComputeMeleeEnvironmentDamage(FName WeaponRow, EHitSurface Surface, ETeamRelation Relation, float& OutDamage);
}
