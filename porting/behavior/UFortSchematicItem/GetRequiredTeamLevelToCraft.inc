// CONFIDENCE: HIGH
// Clamps and writes cached RequiredTeamLevel +0x12c to zero when negative.
if (RequiredTeamLevel < 0)
{
    const_cast<UFortSchematicItem*>(this)->RequiredTeamLevel = 0;
}
return RequiredTeamLevel;
