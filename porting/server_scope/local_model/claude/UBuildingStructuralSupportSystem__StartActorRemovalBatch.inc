// CONFIDENCE: MEDIUM. Opens one nesting level of building-removal batching. The depth is only maintained while the batching global is enabled; otherwise the call is a no-op.
// Original addresses: exec 0x14275eaa0 (P_FINISH bookkeeping ignored).
if (GBuildingRemovalBatchingEnabled)
{
	++ActorRemovalBatchDepth;
}
