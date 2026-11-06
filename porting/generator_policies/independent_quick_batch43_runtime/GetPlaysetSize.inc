// CONFIDENCE: HIGH
// Original exec 0x142a58830; native body 0x1419d5f80.
const UFortPlaysetItemDefinition* Playset = CurrentPlayset;
if (!Playset)
{
    SizeX = SizeY = SizeZ = 1;
    return;
}
SizeX = Playset->SizeX;
SizeY = Playset->SizeY;
SizeZ = FMath::Max(Playset->SizeZ, 1);
