// CONFIDENCE: high
// Native 0x142094190; wrapper 0x1429c0f90.
{
    return Round > 0 && Round <= RoundWinHistory.Num() ? RoundWinHistory[Round - 1] : -1;
}
