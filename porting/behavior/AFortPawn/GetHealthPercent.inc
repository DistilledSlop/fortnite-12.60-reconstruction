// CONFIDENCE: high; native queries health before maximum health.
if (IsDead())
{
    return 0.0f;
}
const float Health = GetHealth();
const float Maximum = GetMaxHealth();
return Health / Maximum;
