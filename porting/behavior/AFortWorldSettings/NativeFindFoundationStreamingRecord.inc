// CONFIDENCE: HIGH. Original 0x141fc2cb0; ordered scan and inclusive tolerance.
NativeBuildFoundationStreamingCache();
for (FBuildingFoundationStreamingData& Record : NativeFoundationStreamingRecords_0x820)
{
    if (Record.FoundationName == Foundation.GetFName() &&
        FMath::Abs(Record.FoundationLocation.X - Location.X) <= 1.0f &&
        FMath::Abs(Record.FoundationLocation.Y - Location.Y) <= 1.0f &&
        FMath::Abs(Record.FoundationLocation.Z - Location.Z) <= 1.0f)
        return &Record;
}
return nullptr;
