// CONFIDENCE: HIGH
// ABuildingSMActor* NativeFindWall141c7d9e0(const FIntVector& Cell, uint8 Side) const
if (Cell.X > NativeRecoveredInt32_0x368 || Cell.Y > NativeRecoveredInt32_0x36c || Cell.Z > NativeRecoveredInt32_0x370) return nullptr;
const int32 Key = NativeWallIndex141c7db30(Cell, Side);
if (Key == -1) return nullptr;
const auto& Map = Side > 1 ? NativeWallYActors_0x218 : NativeWallXActors_0x1c8;
return FortStructuralOriginal::ValidBuilding(Map.Find(Key));
