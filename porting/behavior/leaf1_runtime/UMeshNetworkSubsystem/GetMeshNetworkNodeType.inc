// Behaviour snippet: UMeshNetworkSubsystem::GetMeshNetworkNodeType (body only; spliced into the generated member function).
// Original address: 0x14107ab70
// Evidence: asm MOVZX EAX,byte [RCX+0xb0]; RET. Member NodeType (EMeshNetworkNodeType) at +0xb0 (reflection offset 176, UMeshNetworkSubsystem.h).
// CONFIDENCE: HIGH
return NodeType;
