// CONFIDENCE: MEDIUM
// Original: exec thunk 0x142a2a700; helper 0x14202ca00; static empty text 0x1407c2c80 (FText::GetEmpty equivalent).
// The object at +0x2a8 is unnamed in the headers; its virtual at slot 0x2e0 writes the display text (target unresolved).
FText Result;
if (UObject* Provider = DisplayNameProvider) // member at AFortPickup+0x2a8
{
    Provider->NativeGetDisplayText(Result, 0); // vslot 0x2e0, unnamed
}
else
{
    Result = FText::GetEmpty();
}
return Result;
