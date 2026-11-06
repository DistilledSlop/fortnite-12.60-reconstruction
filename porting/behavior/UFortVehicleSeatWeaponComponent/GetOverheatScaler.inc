// CONFIDENCE: high; Native checks CachedOverheatValue at +0xec and CachedOverheatMax at +0xe8, then divides value by max when both thresholds pass.
return CachedOverheatValue > 0.0f && CachedOverheatMax > 1.e-8f ? CachedOverheatValue / CachedOverheatMax : 0.0f;
