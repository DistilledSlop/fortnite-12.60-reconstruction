// Behaviour snippet: UFortContentControlsManager::SetVoiceChatSettings (body only; spliced into the generated member function).
// Original address: 0x142176710
// Offset evidence: MOV word ptr [RCX+0xd9] with immediate 0x0 (InSetting 0), 0x1 (InSetting 1), 0x101 (InSetting 2); no store otherwise. Object offset 0xd9 is below the class's lowest own reflected offset (0x120), so raw access.
// CONFIDENCE: MEDIUM
uint8* Raw = reinterpret_cast<uint8*>(this);
const uint8 Setting = static_cast<uint8>(InSetting);
if (Setting == 0)
{
	*reinterpret_cast<uint16*>(Raw + 0xd9) = 0x0000;
}
else if (Setting == 1)
{
	*reinterpret_cast<uint16*>(Raw + 0xd9) = 0x0001;
}
else if (Setting == 2)
{
	*reinterpret_cast<uint16*>(Raw + 0xd9) = 0x0101;
}
