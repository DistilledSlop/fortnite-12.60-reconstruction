#pragma once

#include "CoreMinimal.h"

struct FFortPlayerAttributeSets;
struct FFortPlayerPawnStats;
class FProperty;
class AFortPlayerState;
struct FGameDifficultyInfo;

namespace FortPlayerStatFallback
{
// Original stat mutation/loading remains a separate unit.
// 0x1424279f0. The set contains the actual FProperty pointers returned by hero
// curve initialization, not names or all properties on initialized sets.
void Apply(FFortPlayerAttributeSets& Sets, const FFortPlayerPawnStats& Stats,
           const FGameDifficultyInfo* Context, const TSet<FProperty*>& InitializedProperties);

// 0x142427930, including optional zone difficulty lookup.
void ApplyForPlayerState(AFortPlayerState& PlayerState, const FFortPlayerPawnStats& Stats,
                         const TSet<FProperty*>& InitializedProperties);
}
