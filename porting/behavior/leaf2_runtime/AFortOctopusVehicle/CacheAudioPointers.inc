// Behaviour snippet: AFortOctopusVehicle::CacheAudioPointers (body only; spliced into the generated member function).
// Original body address: 0x1419c2870 (thunk 0x142a08620)
// Offset evidence: Stores RDX/R8/R9 (params 1-3) at this+0x1640/0x1648/0x1650 = CacheAudioMovement/CacheAudioWind/CacheAudioTowCable (reflection offsets 5696/5704/5712).
// CONFIDENCE: MEDIUM

CacheAudioMovement = InAudioTowCable;
CacheAudioWind = InAudioWind;
CacheAudioTowCable = InAudioMovement;
