// CONFIDENCE: LOW - static function. The exec 0x14279d3f0 steps the WorldContext (offset 0) and AI (offset 8) parameters.
// Original addresses: 0x14279d3f0 (exec thunk) ; 0x1416ac190 (direct wrapper) ; 0x141702e20 (helper)
// Wrapper 0x1416ac190 ignores WorldContext and passes AI to helper 0x141702e20, which the port declares as SetCurrentGoalDiscouragementImpl.
if (AI == nullptr)
{
    return;
}
SetCurrentGoalDiscouragementImpl(AI);
