// CONFIDENCE: high; recovered native calculation, GetCurrentControlState dependency, and ParticipantBestTimes array indexing from 0x141ad8fb0.
const uint8 Index = static_cast<uint8>(GetCurrentControlState()) * 6 + static_cast<uint8>(StatType);
if (!ParticipantBestTimes.IsValidIndex(Index))
{
	return false;
}
OutStatValue = ParticipantBestTimes[Index];
return true;
