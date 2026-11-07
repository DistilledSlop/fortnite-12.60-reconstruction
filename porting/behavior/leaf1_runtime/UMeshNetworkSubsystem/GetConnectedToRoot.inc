// Behaviour snippet: UMeshNetworkSubsystem::GetConnectedToRoot (body only; spliced into the generated member function).
// Original address: 0x14107a310
// Evidence: asm MOVZX EAX,byte [RCX+0xb2]; RET (whole byte, no mask). Member bConnectedToRoot is a plain bool at +0xb2 (bool_bits.json mov8 mask 1, offset 178; UMeshNetworkSubsystem.h line 54).
// CONFIDENCE: HIGH
return bConnectedToRoot;
