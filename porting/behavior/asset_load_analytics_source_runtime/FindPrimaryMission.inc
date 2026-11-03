// CONFIDENCE: HIGH
for (AFortMission* Mission : Missions)
{
    if (Mission)
    {
        const UFortMissionInfo* Info = Mission->MissionInfo;
        if (Info && Info->MissionType == EFortMissionType::Primary)
        {
            return Mission;
        }
    }
}
return nullptr;
