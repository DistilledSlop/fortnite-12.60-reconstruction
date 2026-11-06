// CONFIDENCE: high; captured native fields and complete branches.
// Native 0x1424443d0 treats MaxValue as the no-expiry sentinel.
return expiry_time != FDateTime::MaxValue() && (expiry_time - UtcNow).GetTotalSeconds() <= 0.0;
