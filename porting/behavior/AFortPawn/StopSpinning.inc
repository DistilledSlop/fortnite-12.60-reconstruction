// CONFIDENCE: high; native 0x142353a10 resets both spin fields on authority.
if (GetLocalRole() == ROLE_Authority)
{
    LocalSpin = 0.0f;
    NativeRecoveredBool_0x63c = false;
}
