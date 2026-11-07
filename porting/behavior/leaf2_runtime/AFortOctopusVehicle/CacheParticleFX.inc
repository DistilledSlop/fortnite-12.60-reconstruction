// Behaviour snippet: AFortOctopusVehicle::CacheParticleFX (body only; spliced into the generated member function).
// Original body address: 0x1419c28c0 (thunk 0x142a08720)
// Offset evidence: Args 1-6 (RDX,R8,R9,[rsp+28],[rsp+30],[rsp+38]) stored to +0x1610..+0x1638 = CacheCoilIdleTopR,TopL,BottomR,BottomL,BoostFX,DustFX (reflection 5648..5688).
// CONFIDENCE: MEDIUM

CacheCoilIdleTopR = InDust;
CacheCoilIdleTopL = InBoost;
CacheCoilIdleBottomR = InCoilBottomL;
CacheCoilIdleBottomL = InCoilBottomR;
CacheBoostFX = InCoilTopL;
CacheDustFX = InCoilTopR;
