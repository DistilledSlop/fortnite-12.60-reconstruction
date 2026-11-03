// CONFIDENCE: high; captured native behavior and reflected types.
// Native 0x142323a90: dead pawns return zero; positive shield takes priority.
if (IsDead())
{
    return 0.0f;
}
const float Shield = GetShield();
if (Shield > 0.0f)
{
    return Shield / GetMaxShield();
}
return GetHealth() / GetMaxHealth();
