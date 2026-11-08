// Original body 0x141c77d20 (thunk 0x14275c490) for UBuildingStructuralSupportSystem::GetGridBox.
// Offset evidence: [RCX+0x378/0x37c/0x380] unreflected own-region floats = grid origin (member_fields NativeRecoveredFloat_0x378/0x37c/0x380); CellIndex struct pointer in R8 (X,Y,Z dwords at +0/+4/+8); FBox sret in RDX (Min, Max, IsValid byte at +0x18).
// Cell sizes 512/512/384 are .data snapshot values of 0x1478c3480/84/88 used as literals; half extents are function-local statics (0.5 * size, constant 0x1459a86f0). MSVC static guard omitted. Confidence MEDIUM for the global constants.
// CONFIDENCE: MEDIUM
const float GridSizeX = 512.0f;
const float GridSizeY = 512.0f;
const float GridSizeZ = 384.0f;
static const float HalfExtentX = GridSizeX * 0.5f;
static const float HalfExtentY = GridSizeY * 0.5f;
static const float HalfExtentZ = GridSizeZ * 0.5f;
const float CenterX = static_cast<float>(CellIndex.X) * GridSizeX + NativeRecoveredFloat_0x378;
const float CenterY = static_cast<float>(CellIndex.Y) * GridSizeY + NativeRecoveredFloat_0x37c;
const float CenterZ = (static_cast<float>(CellIndex.Z) * GridSizeZ + HalfExtentZ) + NativeRecoveredFloat_0x380;
return FBox(FVector(CenterX - HalfExtentX, CenterY - HalfExtentY, CenterZ - HalfExtentZ), FVector(CenterX + HalfExtentX, CenterY + HalfExtentY, CenterZ + HalfExtentZ));
