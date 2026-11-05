#pragma once

#include "CoreMinimal.h"

// Draft adapter for native FTeamMapInfo storage: pixels at 0x20, count at 0x40.
// The original function signatures and fast-path dispatch ABI are not established.
namespace FogRaster
{
    void DrawGradient(TArray<uint8>& Pixels, int32& TouchedCount, int32 CenterX, int32 CenterY,
        float InnerRadius, float FadeInRadius, float FadeOutRadius, float OuterRadius, uint8 Opacity);
    void DrawFastDisk(TArray<uint8>& Pixels, int32& TouchedCount, int32 CenterX, int32 CenterY,
        float Radius);
}
