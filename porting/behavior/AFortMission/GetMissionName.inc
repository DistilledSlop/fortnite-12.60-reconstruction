// CONFIDENCE: high
// Native 0x1420ba150 returns the mission info's text, or shared empty text.
return MissionInfo ? MissionInfo->MissionName : FText::GetEmpty();
