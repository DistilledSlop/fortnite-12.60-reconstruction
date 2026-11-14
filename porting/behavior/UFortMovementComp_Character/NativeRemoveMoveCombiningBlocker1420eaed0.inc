// CONFIDENCE: HIGH. Original1420eaed0 removes every matching name, swapping and shrinking.
for (int32 Index = 0; Index < NativeMoveCombiningBlockers1c60.Num(); ++Index)
{
    if (NativeMoveCombiningBlockers1c60[Index] == AbilityName)
    {
        NativeMoveCombiningBlockers1c60.RemoveAtSwap(Index, 1, true);
        --Index;
    }
}
