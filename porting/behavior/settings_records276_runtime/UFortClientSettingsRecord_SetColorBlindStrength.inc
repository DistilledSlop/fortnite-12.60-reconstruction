// Native 0x141e3aaf0. CONFIDENCE: high
const float NewStrength = static_cast<float>(FMath::Clamp(InColorBlindStrength, 0, 10));
if (NewStrength != ColorBlindStrength)
{
    ColorBlindStrength = NewStrength;
    bDirty = true;
    FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
        static_cast<EColorVisionDeficiency>(static_cast<uint8>(ColorBlindMode)),
        static_cast<int32>(NewStrength), true, false);
}
