// CONFIDENCE: HIGH
// ABuildingSMActor* NativeFindFloor141c77b40(const FIntVector& Cell, uint8 Below) const
using namespace FortStructuralOriginal;
if (Cell.X > NativeRecoveredInt32_0x368 || Cell.Y > NativeRecoveredInt32_0x36c || Cell.Z > NativeRecoveredInt32_0x370) return nullptr;
int32 Z = Cell.Z;
if (Below == 0) Z = Add(Z,1);
else if (Below != 1) return nullptr;
// Preserve native stride order: SX * Z + Y, multiplied by SY, then +X.
const int32 Key = Add(Mul(Add(Mul(NativeRecoveredInt32_0x368,Z),Cell.Y),NativeRecoveredInt32_0x36c),Cell.X);
return Key == -1 ? nullptr : FortStructuralOriginal::ValidBuilding(NativeFloorActors_0x268.Find(Key));
