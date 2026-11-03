// Native0x142883bb0 CONFIDENCE: high
// Inline body in exec wrapper (no callee): compare/store TouchInteractionMode then mark bDirty.
if (TouchInteractionMode != bNewMode)
{
    TouchInteractionMode = bNewMode;
    bDirty = true;
}
