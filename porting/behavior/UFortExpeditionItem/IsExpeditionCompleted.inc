// Behaviour snippet: UFortExpeditionItem::IsExpeditionCompleted (body only; spliced into the generated member function).
// Original address: 0x1424ebbe0 (via exec wrapper 0x1428e4170)
// Source: local-model fn runner; static checks only, not compiled or runtime verified.
// CONFIDENCE: HIGH
if (expedition_start_time < expedition_end_time)
{
    return (expedition_end_time - UtcNow).GetTotalSeconds() <= 0.0;
}
return false;
