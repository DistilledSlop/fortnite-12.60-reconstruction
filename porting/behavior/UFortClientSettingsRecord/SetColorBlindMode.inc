// Native 0x141e3aaa0. CONFIDENCE: high
if (ColorBlindMode != InMode)
{
    ColorBlindMode = InMode;
    bDirty = true;
    FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
        static_cast<EColorVisionDeficiency>(static_cast<uint8>(InMode)),
        static_cast<int32>(ColorBlindStrength), true, false);
}
