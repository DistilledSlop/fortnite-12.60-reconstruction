// CONFIDENCE: HIGH
// Reviewed against captured wrapper and native code; build and runtime pending.
if (expedition_start_time < expedition_end_time)
{
    return false;
}
return (expedition_expiration_end_time - UtcNow).GetTotalSeconds() <= 0.0;
