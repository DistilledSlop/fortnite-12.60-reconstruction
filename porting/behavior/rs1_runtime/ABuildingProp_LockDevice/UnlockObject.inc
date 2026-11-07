// Behaviour snippet: ABuildingProp_LockDevice::UnlockObject (body only).
// Original body address: 0x1406968f0 (thunk 0x142750f70), receiver delta 0.
// Evidence: the body is LEA RAX,[RCX-0x218]; RET. The only computed value is this-0x218 in RAX, which the void declaration discards. No loads, stores or calls.
// CONFIDENCE: HIGH
return;
