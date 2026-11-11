// CONFIDENCE: MEDIUM - cell math and Top(z+1)/Bottom(z) mapping read in full; the hash-table internals are described in the NativeGetFloorActor dep.
// Original addresses: exec 0x14275c3a0; body 0x141c77b40
FIntVector Cell = GridIndices;
if (FloorPosition == EStructuralFloorPosition::Top)
{
    // The top surface of cell Z is the floor stored one level up.
    Cell.Z += 1;
}
return NativeGetFloorActor(Cell);
