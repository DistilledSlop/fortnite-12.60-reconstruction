// Original address: 0x141c008f0 (ABuildingProp_CaptureArea::CaptureItemsFromPlayer)
// Offset evidence: RAX=[RSP+0x30] is the 6th ABI slot = int32& AddedScore; asm is MOV dword ptr [RAX],0; XOR EAX,EAX; RET.
// CONFIDENCE: HIGH
AddedScore = 0;
return 0;
