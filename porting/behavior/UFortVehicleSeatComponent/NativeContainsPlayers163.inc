// CONFIDENCE: high
// 0x141d02da0, primary-vtable byte offset 0x3e8.
for (int32 Index = 0; Index < PlayerSlots.Num(); ++Index)
{
    if (PlayerSlots[Index].Player)
    {
        return true;
    }
}
return false;
