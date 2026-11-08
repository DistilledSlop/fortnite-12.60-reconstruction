// Behaviour snippet: UFortMcpContext::GetPersonalXpBonusPercent (body only).
// Original address: 0x1421d89d0 (thunk 0x142995850)
// Offset evidence: UGameDataSTW float at object +0xf8 (PersonalBoost_XpBonusPercent, reflected) from helper 0x1424e1120 = UGameDataSTW::NativeGetGameDataSTW(). Multiplied by the float constant 100.0 read from .rdata 0x1459a8728; CVTTSS2SI truncates to int32.
// CONFIDENCE: HIGH
static_assert(sizeof(UGameDataSTW) > 0, "UGameDataSTW must be complete for member access");
const float ScaledBoost = UGameDataSTW::NativeGetGameDataSTW()->PersonalBoost_XpBonusPercent * 100.0f;
return static_cast<int32>(ScaledBoost);
