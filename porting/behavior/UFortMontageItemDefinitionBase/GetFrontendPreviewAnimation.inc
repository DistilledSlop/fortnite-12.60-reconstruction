// CONFIDENCE: high
// Native target 0x141d7e270.
TSoftObjectPtr<UAnimMontage> Result = (static_cast<uint8>(Gender) == 2 && !AnimationFemaleOverride.IsNull()) ? AnimationFemaleOverride : Animation;
for (const FFortEmoteMapping& Override : AnimationOverrides)
{
    if (Override.BodyType == BodyType && Override.Gender == Gender)
    {
        Result = Override.EmoteMontage;
    }
}
return Result;
