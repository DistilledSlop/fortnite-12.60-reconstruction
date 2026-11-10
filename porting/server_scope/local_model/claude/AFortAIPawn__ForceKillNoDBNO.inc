// CONFIDENCE: MEDIUM. Authority-only forced kill for a pawn outside the protected state. Clears the AI flag and the DBNO instigator, then kills through the damage-tag path.
// Original address: 0x1427a47c0 (exec). Native body: virtual slot 0xa80 -> 0x142310c30 (KillWithDamageTag wrapper), which forwards to slot 0xa78 -> 0x142310ce0 (KillWithDamageTags).
if (Role == ROLE_Authority && (ProtectedFlags520 & 0x08) == 0)
{
    AIFlags1344 &= ~0x20;
    LastDBNOSource = nullptr;
    KillWithDamageTag(GetFortGlobals()->DBNOForceKillTag, 0, this);
}
