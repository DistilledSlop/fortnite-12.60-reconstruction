// CONFIDENCE: HIGH
// Original exec 0x142a58830; native body 0x1419d5f80.
if (!CurrentPlayset)
{
    SizeX = SizeY = SizeZ = 1;
    return;
}
SizeX = CurrentPlayset->SizeX;
SizeY = CurrentPlayset->SizeY;
SizeZ = FMath::Max(CurrentPlayset->SizeZ, 1);
