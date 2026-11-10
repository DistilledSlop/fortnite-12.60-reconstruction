// CONFIDENCE: MEDIUM. Stores the new custom state string and runs the post-change hook.
// Original address: 0x1427381a0 (exec). Native body: 0x141bc49a0 (virtual slot 0x818); it resets the stored CustomState at +0x518 and calls 0x142733f90, which is the change notification (treated here as OnRep_CustomState).
CustomState = NewState;
OnRep_CustomState();
