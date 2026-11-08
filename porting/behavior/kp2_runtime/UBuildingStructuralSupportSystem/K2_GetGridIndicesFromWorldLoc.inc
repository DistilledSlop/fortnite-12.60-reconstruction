// Behaviour snippet: UBuildingStructuralSupportSystem::K2_GetGridIndicesFromWorldLoc (body only; spliced into the generated member function).
// Original body address: 0x141c78270 (thunk 0x14275db40)
// Offset evidence: validity bounds as IsWorldLocValid (this+0x384..0x398); grid origin this+0x378 (X), +0x37c (Y), +0x380 (Z) (NativeRecoveredFloat members).
// Scale 1/512 for X and Y, 1/384 for Z (.rdata 0x145d7946c, 0x145d79470); the asm doubles the scaled value, adds 0.5, rounds with CVTSS2SI (nearest-even) and shifts right by 1.
// Outputs are written only when every axis is valid; every asm instruction is accounted for.
// CONFIDENCE: HIGH
const float Epsilon = 9.999999747378752e-05f;
const float X = WorldLoc.X;
const float Y = WorldLoc.Y;
const float Z = WorldLoc.Z;
const bool bXLowOk = (X > NativeRecoveredFloat_0x384) || !(FMath::Abs(X - NativeRecoveredFloat_0x384) > Epsilon);
const bool bXHighOk = !(X > NativeRecoveredFloat_0x390) || !(FMath::Abs(X - NativeRecoveredFloat_0x390) > Epsilon);
const bool bYLowOk = (Y > NativeRecoveredFloat_0x388) || !(FMath::Abs(Y - NativeRecoveredFloat_0x388) > Epsilon);
const bool bYHighOk = !(Y > NativeRecoveredFloat_0x394) || !(FMath::Abs(Y - NativeRecoveredFloat_0x394) > Epsilon);
const bool bZLowOk = (Z > NativeRecoveredFloat_0x38c) || !(FMath::Abs(Z - NativeRecoveredFloat_0x38c) > Epsilon);
const bool bZHighOk = !(Z > NativeRecoveredFloat_0x398) || !(FMath::Abs(Z - NativeRecoveredFloat_0x398) > Epsilon);
if (!(bXLowOk && bXHighOk && bYLowOk && bYHighOk && bZLowOk && bZHighOk))
{
    return false;
}

// Round-half-to-even per CVTSS2SI, then arithmetic shift right by one (SAR), per axis.
float ScaledX = (X - NativeRecoveredFloat_0x378) * 0.001953125f;
ScaledX = ScaledX + ScaledX + 0.5f;
const float FloorX = FMath::FloorToFloat(ScaledX);
int32 IndexX = static_cast<int32>(FloorX);
const float FracX = ScaledX - FloorX;
if (FracX > 0.5f || (FracX == 0.5f && (IndexX & 1) != 0))
{
    IndexX += 1;
}

float ScaledY = (Y - NativeRecoveredFloat_0x37c) * 0.001953125f;
ScaledY = ScaledY + ScaledY + 0.5f;
const float FloorY = FMath::FloorToFloat(ScaledY);
int32 IndexY = static_cast<int32>(FloorY);
const float FracY = ScaledY - FloorY;
if (FracY > 0.5f || (FracY == 0.5f && (IndexY & 1) != 0))
{
    IndexY += 1;
}

float ScaledZ = (Z - NativeRecoveredFloat_0x380) * 0.0026041667442768812f;
ScaledZ = ScaledZ + ScaledZ + 0.5f;
const float FloorZ = FMath::FloorToFloat(ScaledZ);
int32 IndexZ = static_cast<int32>(FloorZ);
const float FracZ = ScaledZ - FloorZ;
if (FracZ > 0.5f || (FracZ == 0.5f && (IndexZ & 1) != 0))
{
    IndexZ += 1;
}

OutGridIndices.X = IndexX >> 1;
OutGridIndices.Y = IndexY >> 1;
OutGridIndices.Z = IndexZ >> 1;
return true;
