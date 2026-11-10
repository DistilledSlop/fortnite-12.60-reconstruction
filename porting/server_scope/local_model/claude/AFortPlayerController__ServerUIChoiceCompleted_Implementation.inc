// CONFIDENCE: MEDIUM
// Original: 0x1423ad150 (virtual Implementation of ServerUIChoiceCompleted). Callees not named here are described in the evidence file.
// Broadcast the UI choice to all bound listeners (controller delegate at +0xc00, lock at +0xbb8).
UIChoiceCompletedDelegate.Broadcast(ChosenItem, MenuIdentifier);
