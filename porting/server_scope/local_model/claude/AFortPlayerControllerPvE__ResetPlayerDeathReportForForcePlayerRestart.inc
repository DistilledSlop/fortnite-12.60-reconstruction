// CONFIDENCE: MEDIUM
// Original address: 0x142a6c270 (exec thunk, no parameters, no return value; body is direct, no callees)
// Clears the forced-restart death report: zeroes the two timestamps at the head of PlayerDeathReport
// (FortPlayerDeathReport +0x0 ServerTimeForRespawn and +0x4 ServerTimeForResurrect, one 8-byte store),
// and clears bit 1 of the 32-bit flags word at PlayerDeathReport +0x38.
// The flags word at +0x38 is not resolved to a reflected member: bDroppedBackpack is a bitfield with no
// bit position in the reflection dump, so the bit is named by its mask here, not by a member name.
PlayerDeathReport.ServerTimeForRespawn = 0.0f;
PlayerDeathReport.ServerTimeForResurrect = 0.0f;
PlayerDeathReport_FlagsWord_0x38 &= ~0x2u;
