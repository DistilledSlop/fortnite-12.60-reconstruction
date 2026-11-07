// Behaviour snippet: AFortAthenaFerretVehicle::CacheAudioVoicePointers (body only; spliced into the generated member function).
// Original body address: 0x1418e44d0 (thunk 0x1427d7890)
// Offset evidence: RDX,R8,R9 = params 0..2; [RSP+28..38] = params 3..5. Stores: 0x1640<-RDX, 0x1648<-R8, 0x1650<-R9, 0x1658<-[RSP+28], 0x1660<-[RSP+30], 0x1668<-[RSP+38].
// CONFIDENCE: MEDIUM
FortAudioMovementClose = InAudioSpark;
FortAudioMovementDistant = InAudioTaxi;
FortAudioWindFirstPerson = InAudioDive;
FortAudioDive = InAudioWindFirstPerson;
FortAudioTaxi = InAudioMovementDistant;
FortAudioSpark = InAudioMovementClose;
