// CONFIDENCE: HIGH
// int32 NativeWallIndex141c7db30(const FIntVector& C, uint8 Side) const
using namespace FortStructuralOriginal;
const int32 SX = NativeRecoveredInt32_0x368, SY = NativeRecoveredInt32_0x36c;
switch (Side) {
case 0: return Add(Mul(Add(Mul(SY,C.Z),C.Y),Add(SX,1)), C.X);
case 1: return Add(Add(C.X,1),Mul(Add(Mul(SY,C.Z),C.Y),Add(SX,1)));
case 2: return Add(Add(C.Y,1),Mul(Add(Mul(SX,C.Z),C.X),Add(SY,1)));
case 3: return Add(Mul(Add(SY,1),Add(Mul(SX,C.Z),C.X)),C.Y);
default: return -1;
}
