// CONFIDENCE: high
// Native target 0x1420088a0.
UAnimMontage* Animation = GetUseAnimation();
if (Animation && UseTime > 1.e-8f)
    return Animation->SequenceLength / UseTime;
return 1.0f;
