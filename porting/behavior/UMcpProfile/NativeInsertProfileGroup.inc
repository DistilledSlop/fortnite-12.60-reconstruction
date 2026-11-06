// CONFIDENCE: HIGH
// The original compares UTF-16 units, folding ASCII only when both differ
// and are ASCII. Unequal non-ASCII units compare directly.
const auto CompareIds = [](const FString& Left, const FString& Right) -> int32
{
    const TCHAR* A = *Left;
    const TCHAR* B = *Right;
    for (;; ++A, ++B)
    {
        uint32 X = static_cast<uint32>(*A);
        uint32 Y = static_cast<uint32>(*B);
        if (X == Y)
        {
            if (X == 0) return 0;
            continue;
        }
        if ((X | Y) < 0x80)
        {
            if (X >= 'A' && X <= 'Z') X += 'a' - 'A';
            if (Y >= 'A' && Y <= 'Z') Y += 'a' - 'A';
        }
        if (X != Y) return X < Y ? -1 : 1;
    }
};
FMcpProfileItemArray& Group = Groups.FindOrAdd(Key);
int32 First = 0;
int32 Count = Group.Num();
while (Count > 0)
{
    const int32 Half = Count / 2;
    const int32 Mid = First + Half;
    if (CompareIds(Group[Mid]->ItemId, Item->ItemId) < 0)
    {
        First = Mid + 1;
        Count -= Half + 1;
    }
    else
    {
        Count = Half;
    }
}
if (First == Group.Num() || CompareIds(Group[First]->ItemId, Item->ItemId) != 0)
{
    Group.Insert(Item, First);
}
