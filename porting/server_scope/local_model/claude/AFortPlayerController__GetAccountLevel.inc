// CONFIDENCE: MEDIUM - JSON lookup key (DAT_148164eb8) is not resolved to a string; the number conversion is fully read.
// Original address: 0x142a46f70 (exec thunk). Helper: 0x14237cc60. Lookup: 0x1410ff2d0 (virtual slot 0x40 on profile sub-object). Number read: 0x142cf2280.
// Returns the "Level" field of the main MCP profile JSON as an int32, or 0 when the profile or field is missing.
if (MainMcpProfile == nullptr)
{
	return 0;
}
TSharedPtr<FJsonValue> LevelValue = MainMcpProfile->GetJsonField_0x1410ff2d0(FieldKey_0x148164eb8);
if (!LevelValue.IsValid())
{
	return 0;
}
// 0x142cf2280: TryGetNumber; on type mismatch it logs "Json Value of type '%s' used as a 'Number'." and yields 0.
double LevelNumber = LevelValue->AsNumber();
return (int32)LevelNumber;
