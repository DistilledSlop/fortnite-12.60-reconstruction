// Behaviour snippet: UFortMcpContext::GetRestXpBonusPercent (body only).
// Original address: 0x1421d9590 (thunk 0x142995a40)
// Offset evidence: UGameDataSTW float at object +0x104 (Rest_XpBonusPercent, reflected) from helper 0x1424e1120 = UGameDataSTW::NativeGetGameDataSTW(). Multiplied by the float constant 100.0 read from .rdata 0x1459a8728; CVTTSS2SI truncates to int32.
// CONFIDENCE: HIGH
static_assert(sizeof(UGameDataSTW) > 0, "UGameDataSTW must be complete for member access");
const float ScaledBoost = UGameDataSTW::NativeGetGameDataSTW()->Rest_XpBonusPercent * 100.0f;
return static_cast<int32>(ScaledBoost);
