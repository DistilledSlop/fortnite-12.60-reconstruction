// Original body address: 0x1423aa2e0
// Offset evidence: ADD RCX,0x1b58 selects the reflected member OnPlayerChangedBuildMode (InlineMulticastDelegate, FOnPlayerEnteredBuildMode with one bool). The bool is spilled to a stack parameter block (MOV byte [RSP+0x38],DL) and broadcast via CALL 0x1406a9dc0.
// CONFIDENCE: HIGH
OnPlayerChangedBuildMode.Broadcast(bIsInBuildMode);
