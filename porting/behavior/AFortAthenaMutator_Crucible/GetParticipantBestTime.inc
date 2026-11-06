// CONFIDENCE: high; recovered native body and reflected field mapping.
const uint8 Index = static_cast<uint8>(StatId);
if (!ParticipantBestTimes.IsValidIndex(Index))
{
    return false;
}
OutStatValue = ParticipantBestTimes[Index];
return true;
