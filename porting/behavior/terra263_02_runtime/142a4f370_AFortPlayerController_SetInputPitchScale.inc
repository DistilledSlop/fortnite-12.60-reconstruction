// Native 0x142a4f370; CONFIDENCE: high.
const float Sign = InputPitchScale > 0.0f ? 1.0f : (InputPitchScale < 0.0f ? -1.0f : 0.0f);
InputPitchScale = FMath::Abs(InInputPitchScale) * Sign;
