// CONFIDENCE: medium
// Original address: 0x142bad740
// Reason: Unnamed global _DAT_147c306f0 used; LOCK/UNLOCK atomicity is not modeled, plain statements kept.
int64 lVar1 = _DAT_147c306f0 + 1;
int64 lVar2 = _DAT_147c306f0 + 1;
if (lVar2 == 0)
{
    // decompile LOCK/UNLOCK pair here; atomic ordering not modeled
    lVar2 = _DAT_147c306f0 + 2;
    lVar1 = _DAT_147c306f0 + 2;
}
_DAT_147c306f0 = lVar1;
return lVar2;
