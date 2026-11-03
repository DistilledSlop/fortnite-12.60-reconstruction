// Native 0x142a4f3f0; CONFIDENCE: high.
const float Sign = InputYawScale > 0.0f ? 1.0f : (InputYawScale < 0.0f ? -1.0f : 0.0f);
InputYawScale = FMath::Abs(InInputYawScale) * Sign;
