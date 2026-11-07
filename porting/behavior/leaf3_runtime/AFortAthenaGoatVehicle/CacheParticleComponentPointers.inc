// Behaviour snippet: AFortAthenaGoatVehicle::CacheParticleComponentPointers (body only; spliced into the generated member function).
// Original body address: 0x1418e4550 (thunk 0x1427dabd0)
// Offset evidence: RDX,R8,R9 = params 0..2; [RSP+28] = param 3. Stores: 0x10f0<-RDX, 0x10f8<-R8, 0x1100<-R9, 0x1108<-[RSP+28].
// CONFIDENCE: MEDIUM
PSC_WheelDust = TrailRight;
PSC_WheelSlide = TrailLeft;
PSC_TrailLeft = WheelSlide;
PSC_TrailRight = WheelDust;
