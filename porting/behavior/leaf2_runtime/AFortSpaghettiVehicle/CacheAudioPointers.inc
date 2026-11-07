// Behaviour snippet: AFortSpaghettiVehicle::CacheAudioPointers (body only; spliced into the generated member function).
// Original body address: 0x1419c2870 (thunk 0x142a08620)
// Offset evidence: Same body as AFortOctopusVehicle::CacheAudioPointers; +0x1640/+0x1648/+0x1650 = CacheAudioMovement/Wind/TowCable (reflection 5696/5704/5712).
// CONFIDENCE: MEDIUM

CacheAudioMovement = InAudioTowCable;
CacheAudioWind = InAudioWind;
CacheAudioTowCable = InAudioMovement;
