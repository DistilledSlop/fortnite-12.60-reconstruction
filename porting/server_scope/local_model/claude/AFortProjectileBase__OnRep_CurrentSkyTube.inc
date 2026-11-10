// CONFIDENCE: MEDIUM. Replication notify for CurrentSkyTube. Does nothing unless a tube is set and the projectile's simulation flag (+0x6F4 bit 0) is on. If the actor has the 0x2 flag (+0x32) and no component with flags 0x30, it clears the registered flag on the tick sub-object at +0x28.
// Original address: 0x142aa6290 (no params; no thunk).
if (CurrentSkyTube == nullptr || !(ProjectileFlags & 1))
{
    return;
}
if (RootFlags & 2)
{
    if (!HasComponentWithFlags(0x30))            // 0x142ecc600
    {
        SetRegisteredFlag(PrimaryActorTick, true);  // 0x144630410 on this+0x28, value 1
    }
}
