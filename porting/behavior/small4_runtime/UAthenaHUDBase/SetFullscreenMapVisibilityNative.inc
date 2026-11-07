// Original address: 0x1406a7c40
// Evidence: body is LEA RAX,[RCX-0x108]; RET only. No store, call or branch; the computed pointer is discarded by a void return and bIsVisible (DL) is unused.
// CONFIDENCE: MEDIUM
return;
