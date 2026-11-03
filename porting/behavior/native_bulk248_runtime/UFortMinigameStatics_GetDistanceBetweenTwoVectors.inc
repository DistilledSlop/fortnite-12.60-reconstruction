// Native 0x14246aef0. CONFIDENCE: high
const float X = Source.X - Dest.X;
const float Y = Source.Y - Dest.Y;
const float Z = Source.Z - Dest.Z;
return FMath::Sqrt(Y * Y + X * X + Z * Z);
