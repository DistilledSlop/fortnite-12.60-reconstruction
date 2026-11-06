// CONFIDENCE: high
// 0x141d078a0; PlayerSlots is the reflected FAthenaCarPlayerSlot array at 0xc8.
TArray<APawn*> Result;
for (int32 Index = 0; Index < PlayerSlots.Num(); ++Index)
{
    AFortPlayerPawn* Player = PlayerSlots[Index].Player;
    if (Player)
    {
        Result.Add(Player);
    }
}
return Result;
