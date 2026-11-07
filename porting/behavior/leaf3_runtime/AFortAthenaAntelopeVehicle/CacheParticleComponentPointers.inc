// Behaviour snippet: AFortAthenaAntelopeVehicle::CacheParticleComponentPointers (body only; spliced into the generated member function).
// Original body address: 0x1418bc3b0 (thunk 0x1427cf770)
// Offset evidence: RDX,R8,R9 = params 0..2; [RSP+28],[RSP+30] = params 3,4. Stores: 0x1168<-RDX, 0x1170<-R8, 0x1178<-R9, 0x1180<-[RSP+28], 0x10d8<-[RSP+30].
// CONFIDENCE: MEDIUM
PSC_WheelDust = InEngineReady;
PSC_WheelSlide = TrailRight;
PSC_TrailLeft = TrailLeft;
PSC_TrailRight = WheelSlide;
PSC_ATV_Engine_Ready = WheelDust;
