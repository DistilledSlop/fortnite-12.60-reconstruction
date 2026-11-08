// Original body address: 0x1426819d0 (thunk 0x1428c53b0)
// Offset evidence: MOVZX R8D,byte [RCX+0xf78] is the low byte of RotationAxes.Num() (TArray at 0xf70, Num at +8). DEC R8B gives Num-1 as uint8; CMP DL,R8B + CMOVC keeps AxisIndex when AxisIndex < R8B, else R8B; byte stored to RotationAxisIndex (0xf80, reflected uint8).
// CONFIDENCE: HIGH
const uint8 LastAxisIndex = static_cast<uint8>(static_cast<uint8>(RotationAxes.Num()) - 1);
RotationAxisIndex = (AxisIndex < LastAxisIndex) ? AxisIndex : LastAxisIndex;
