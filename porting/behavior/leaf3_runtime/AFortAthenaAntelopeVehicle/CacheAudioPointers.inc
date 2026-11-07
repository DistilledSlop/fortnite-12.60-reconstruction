// Behaviour snippet: AFortAthenaAntelopeVehicle::CacheAudioPointers (body only; spliced into the generated member function).
// Original body address: 0x1418bc310 (thunk 0x1427cf4b0)
// Offset evidence: ABI args RDX,R8,R9 = params 0..2; [RSP+28..48] = params 3..7. Stores land at reflected CachedAudio* slots 0x1080..0x10b8 (ABI order, not name order).
// Mapping: p0->0x1080 p1->0x1088 p2->0x1090 p3->0x1098 p4->0x10a0 p5->0x10a8 p6->0x10b0 p7->0x10b8.
// CONFIDENCE: MEDIUM
CachedAudioEngineRevUp = InAudioEngineIdle;
CachedAudioSkid = InAudioBoostCharge;
CachedAudioScrape = InAudioHandbrakeSkid;
CachedAudioWind = InAudioBoost;
CachedAudioBoost = InAudioWind;
CachedAudioHandbrakeSkid = InAudioScrape;
CachedAudioBoostCharge = InAudioSkid;
CachedAudioEngineIdle = InAudioEngineRevUp;
