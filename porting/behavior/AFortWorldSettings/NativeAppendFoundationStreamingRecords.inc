// CONFIDENCE: HIGH. Original 0x141fbb310; deep copy and rebase child indices.
const int32 BaseIndex = NativeFoundationStreamingRecords_0x820.Num();
for (const FBuildingFoundationStreamingData& SourceRecord : Source.BuildingFoundationStreamingData)
{
    FBuildingFoundationStreamingData Record = SourceRecord;
    for (int32& ChildIndex : Record.ChildStreamingDataIndices)
        ChildIndex += BaseIndex;
    NativeFoundationStreamingRecords_0x820.Add(MoveTemp(Record));
}
