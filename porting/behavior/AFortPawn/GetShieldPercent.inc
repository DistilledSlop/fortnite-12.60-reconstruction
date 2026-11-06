// CONFIDENCE: high; preserve the repeated maximum-shield virtual query.
if (IsDead())
{
    return 0.0f;
}
if (GetMaxShield() > 0.0f)
{
    const float Shield = GetShield();
    const float Maximum = GetMaxShield();
    return Shield / Maximum;
}
return 0.0f;
