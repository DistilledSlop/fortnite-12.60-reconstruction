// Behaviour snippet: AFortMeatballVehicle::CachePSCPointers (body only; spliced into the generated member function).
// Original address: 0x1419c2890 (thunk 0x1429b21b0).
// RDX(InCacheSnowLandscapeComponent)->+0x1048 CacheBoostReadyLFx; R8(InCacheDirtCascade)->+0x1050 CacheBoostReadyRFx;
// R9(InCacheBoostReadyRFx)->+0x1058 CacheDirtCascade; [RSP+0x28](InCacheBoostReadyLFx)->+0x1060 CacheSnowInteractionComponent.
// CONFIDENCE: HIGH
CacheBoostReadyLFx = (UParticleSystemComponent*)InCacheSnowLandscapeComponent;
CacheBoostReadyRFx = (UParticleSystemComponent*)InCacheDirtCascade;
CacheDirtCascade = (UParticleSystemComponent*)InCacheBoostReadyRFx;
CacheSnowInteractionComponent = (UNiagaraComponent*)InCacheBoostReadyLFx;
