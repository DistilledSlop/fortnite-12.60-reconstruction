// CONFIDENCE: HIGH
// Native 0x141efa100: four successive quarter-turns, with a strict dot threshold.
FVector Axis(1.0f, 0.0f, 0.0f);
for (int32 I = 0; I < 4; ++I)
{
    if (InVector.X * Axis.X + InVector.Y * Axis.Y + InVector.Z * Axis.Z > 0.7071)
        return Axis;
    const float X = Axis.X;
    const float Y = Axis.Y;
    const float Z = Axis.Z;
    // Preserve the original rotation matrix operations, including zero products.
    Axis.X = -1.1920929e-7f * X + (0.0f - 1.0f) * Y + (0.0f + 0.0f) * Z;
    Axis.Y = -1.1920929e-7f * Y + (0.0f + 1.0f) * X + (0.0f - 0.0f) * Z;
    Axis.Z = (1.0000001f - 1.1920929e-7f) * Z + (0.0f - 0.0f) * X + (0.0f + 0.0f) * Y;
}
return InVector;
