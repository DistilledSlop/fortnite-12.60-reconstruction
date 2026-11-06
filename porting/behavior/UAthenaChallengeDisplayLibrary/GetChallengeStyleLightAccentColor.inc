// Native 0x144b75120. CONFIDENCE: high
const FLinearColor& Color = DisplayStyle.AccentColor;
return FLinearColor((1.0f - Color.R) * 0.3f + Color.R,
    (1.0f - Color.G) * 0.3f + Color.G,
    (1.0f - Color.B) * 0.3f + Color.B,
    (1.0f - Color.A) * 0.3f + Color.A);
