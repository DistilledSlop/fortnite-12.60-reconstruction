// Original body: 0x141d959a0 (thunk 0x142766c90). Offset evidence: CALL 0x1440b2120 = UFortMinigameObjectiveComponent::NativeHelper1440b2120(this) returns a byte compared against 3 (CMP EAX,0x3; JL skips the store). Otherwise CaptureState (reflected ECaptureState at +0x244) is read: 1, or (CaptureState-4) as a byte <= 2, stores 0 to SmoothProgressState (+0x24c); any other value stores 2.
// CONFIDENCE: HIGH
const uint8 HelperResult = UFortMinigameObjectiveComponent::NativeHelper1440b2120(this);
if (HelperResult < 3)
{
    return;
}

const uint8 CaptureStateValue = static_cast<uint8>(CaptureState);
if (CaptureStateValue == 1 || static_cast<uint8>(CaptureStateValue - 4) <= 2)
{
    SmoothProgressState = static_cast<ESmoothProgressState>(0);
}
else
{
    SmoothProgressState = static_cast<ESmoothProgressState>(2);
}
