// CONFIDENCE: HIGH. Original 0x1426cc820 converts the unsigned byte to yaw.
return FRotator(0.0f, float(uint32(uint8(NumRotations)) * 90u), 0.0f);
