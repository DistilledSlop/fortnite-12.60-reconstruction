// Behaviour snippet: AFortMeatballVehicle::CacheAudioPointers (body only; spliced into the generated member function).
// Original address: 0x1419c2840 (thunk 0x1429b2020).
// Parameter-to-member mapping taken from the Windows x64 ABI position (asm authoritative):
// RDX(InAudioScrape)->+0x1018 CacheAudioEngineUp; R8(InAudioWakeSpeed)->+0x1020 CacheAudioEngineDown;
// R9(InAudioWakeTurn)->+0x1028 CacheAudioWakeTurn; [RSP+0x28](InAudioEngineDown)->+0x1030 CacheAudioWakeSpeed;
// [RSP+0x30](InAudioEngineUp)->+0x1038 CacheAudioScrape. All five members are reflected at these offsets.
// CONFIDENCE: HIGH
CacheAudioEngineUp = (UFortVehicleAudioVoice*)InAudioScrape;
CacheAudioEngineDown = (UFortVehicleAudioVoice*)InAudioWakeSpeed;
CacheAudioWakeTurn = (UFortVehicleAudioVoice*)InAudioWakeTurn;
CacheAudioWakeSpeed = (UFortVehicleAudioVoice*)InAudioEngineDown;
CacheAudioScrape = (UFortVehicleAudioVoice*)InAudioEngineUp;
