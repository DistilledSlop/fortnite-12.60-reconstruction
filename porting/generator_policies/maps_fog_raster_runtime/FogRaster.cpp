#include "FogRaster.h"

// These are behavioral helpers, not declarations of the original native ABI.
// As in the original, callers must provide storage for 256 * 256 pixels and
// valid radii/coordinates for the native integer and floating point operations.
namespace FogRaster
{
    void DrawGradient(TArray<uint8>& Pixels, int32& TouchedCount, int32 CenterX, int32 CenterY,
        float InnerRadius, float FadeInRadius, float FadeOutRadius, float OuterRadius, uint8 Opacity)
    {
        const int32 Offset = FMath::FloorToInt(-OuterRadius);
        const int32 MinX = FMath::Max(CenterX + Offset, 0);
        const int32 MinY = FMath::Max(CenterY + Offset, 0);
        const int32 MaxX = FMath::Min(CenterX - Offset, 255);
        const int32 MaxY = FMath::Min(CenterY - Offset, 255);
        for (int32 Y = MinY; Y <= MaxY; ++Y)
        {
            for (int32 X = MinX; X <= MaxX; ++X)
            {
                const int32 DX = CenterX - X;
                const int32 DY = CenterY - Y;
                const float DistanceSquared = float(DX * DX + DY * DY);
                if (DistanceSquared < OuterRadius * OuterRadius)
                {
                    uint8& Pixel = Pixels[Y * 256 + X];
                    TouchedCount += Pixel == 0;
                    uint8 Value;
                    if (DistanceSquared < InnerRadius * InnerRadius)
                    {
                        Value = 255;
                    }
                    else if (DistanceSquared < FadeInRadius * FadeInRadius)
                    {
                        const float Alpha = (FMath::Sqrt(DistanceSquared) - InnerRadius) / (FadeInRadius - InnerRadius);
                        Value = uint8(int32(Alpha * float(int32(Opacity) - 255) + 255.0f) & 0xff);
                    }
                    else if (DistanceSquared < FadeOutRadius * FadeOutRadius)
                    {
                        Value = Opacity;
                    }
                    else
                    {
                        const float Alpha = (FMath::Sqrt(DistanceSquared) - FadeOutRadius) / (OuterRadius - FadeOutRadius);
                        Value = uint8(int32(float(Opacity) - Alpha * float(Opacity)) & 0xff);
                    }
                    Pixel = FMath::Max(Pixel, Value);
                }
            }
        }
    }

    void DrawFastDisk(TArray<uint8>& Pixels, int32& TouchedCount, int32 CenterX, int32 CenterY,
        float Radius)
    {
        const int32 Offset = FMath::FloorToInt(-Radius);
        const int32 IntegerRadius = -Offset;
        const int32 MinX = FMath::Max(CenterX + Offset, 0);
        const int32 MinY = FMath::Max(CenterY + Offset, 0);
        const int32 MaxX = FMath::Min(CenterX + IntegerRadius, 255);
        const int32 MaxY = FMath::Min(CenterY + IntegerRadius, 255);
        for (int32 Y = MinY; Y <= MaxY; ++Y)
        {
            for (int32 X = MinX; X <= MaxX; ++X)
            {
                const int32 DX = CenterX - X;
                const int32 DY = CenterY - Y;
                if (DX * DX + DY * DY < IntegerRadius * IntegerRadius)
                {
                    uint8& Pixel = Pixels[Y * 256 + X];
                    TouchedCount += Pixel == 0;
                    Pixel = 255;
                }
            }
        }
    }
}
