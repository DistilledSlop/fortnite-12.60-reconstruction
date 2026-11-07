// Behaviour snippet: AFortAthenaSKPushVehicle::CacheAudioPointers (body only; spliced into the generated member function).
// Original body address: 0x1418e4490 (thunk 0x142830ea0)
// Offset evidence: RDX,R8,R9 = params 0..2; [RSP+28..38] = params 3..5. Stores: 0xfe0<-RDX, 0xfe8<-R8, 0xff0<-R9, 0xff8<-[RSP+28], 0x1000<-[RSP+30], 0x1008<-[RSP+38].
// CONFIDENCE: MEDIUM
CacheAudioMovement = InAudioWind;
CacheAudioCoast = InAudioScrape;
CacheAudioSkid = InAudioInAir;
CacheAudioInAir = InAudioSkid;
CacheAudioScrape = InAudioCoast;
CacheAudioWind = InAudioMovement;
