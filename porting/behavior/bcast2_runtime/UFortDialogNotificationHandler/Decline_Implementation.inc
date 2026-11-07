// Behaviour snippet: UFortDialogNotificationHandler::Decline_Implementation (body only; spliced into the generated member function).
// Original body: 0x142523410 (thunk 0x1428dba90); native body is a broadcast through ProcessMulticastDelegate 0x1406a9dc0.
// Offset evidence:
//   Asm: ADD RCX,0x50; XOR EDX,EDX -> OnDeclined (+0x50), FOnActionComplete declares no params.
// CONFIDENCE: HIGH
OnDeclined.Broadcast();
