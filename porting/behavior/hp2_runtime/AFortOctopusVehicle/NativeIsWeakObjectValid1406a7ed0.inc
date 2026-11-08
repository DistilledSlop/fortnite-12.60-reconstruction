// Original body address: 0x1406a7ed0 (thunk: JMP 0x142ee7880, TWeakObjectPtr::IsValid)
// Offset evidence: RCX is the TWeakObjectPtr itself (no displacement). Callers pass a weak field (UFortItem::IsActivatable passes [RSI+0x20] and then calls Get on the same field).
// Thunk forwards RDX (bEvenIfPendingKill) unchanged; both callers set RDX to 0. R8 (bThreadsafeTest) is not set by callers, so the port uses the default false.
// CONFIDENCE: MEDIUM
return WeakObject->IsValid(bEvenIfPendingKill);
