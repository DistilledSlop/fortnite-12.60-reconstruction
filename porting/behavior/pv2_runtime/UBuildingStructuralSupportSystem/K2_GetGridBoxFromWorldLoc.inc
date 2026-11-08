// Original body 0x141c77ec0 (thunk 0x14275da80) for UBuildingStructuralSupportSystem::K2_GetGridBoxFromWorldLoc.
// Offset evidence: [RCX+0x378/0x37c/0x380] unreflected own-region floats = grid origin (member_fields NativeRecoveredFloat_0x378/0x37c/0x380); WorldLocation pointer in R8; FBox sret in RDX.
// X,Y: (d*(1/512)); F+F+0.5 -> CVTSS2SI >>1 = FMath::RoundToInt; Z: d*(1/384), F+F-0.5 -> CVTSS2SI >>1 = FMath::FloorToInt. Sizes 512/512/384 are .data snapshot values used as literals; MSVC static guard omitted.
// CONFIDENCE: MEDIUM
const float GridSizeX = 512.0f;
const float GridSizeY = 512.0f;
const float GridSizeZ = 384.0f;
static const float HalfExtentX = GridSizeX * 0.5f;
static const float HalfExtentY = GridSizeY * 0.5f;
static const float HalfExtentZ = GridSizeZ * 0.5f;
const int32 CellX = FMath::RoundToInt((WorldLocation.X - NativeRecoveredFloat_0x378) * 0.001953125f);
const int32 CellY = FMath::RoundToInt((WorldLocation.Y - NativeRecoveredFloat_0x37c) * 0.001953125f);
const int32 CellZ = FMath::FloorToInt((WorldLocation.Z - NativeRecoveredFloat_0x380) * 0.0026041667f);
const float CenterX = static_cast<float>(CellX) * GridSizeX + NativeRecoveredFloat_0x378;
const float CenterY = static_cast<float>(CellY) * GridSizeY + NativeRecoveredFloat_0x37c;
const float CenterZ = (static_cast<float>(CellZ) * GridSizeZ + HalfExtentZ) + NativeRecoveredFloat_0x380;
return FBox(FVector(CenterX - HalfExtentX, CenterY - HalfExtentY, CenterZ - HalfExtentZ), FVector(CenterX + HalfExtentX, CenterY + HalfExtentY, CenterZ + HalfExtentZ));
