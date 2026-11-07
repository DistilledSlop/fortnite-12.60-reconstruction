// Behaviour snippet: UMeshNetworkSubsystem::GetGameServerNodeType (body only; spliced into the generated member function).
// Original address: 0x14107a7e0
// Evidence: asm MOVZX EAX,byte [RCX+0xb1]; RET. Member GameServerNodeType (EMeshNetworkNodeType) at +0xb1 (reflection offset 177, UMeshNetworkSubsystem.h).
// CONFIDENCE: HIGH
return GameServerNodeType;
