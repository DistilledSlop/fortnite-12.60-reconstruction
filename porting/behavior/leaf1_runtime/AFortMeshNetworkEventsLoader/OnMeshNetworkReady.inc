// Behaviour snippet: AFortMeshNetworkEventsLoader::OnMeshNetworkReady (body only; spliced into the generated member function).
// Original address: 0x140777590
// Evidence: asm MOV byte [RCX+0x220],1; RET. bMeshNetworkReady is bool at +0x220 (bool_bits offset 544 mask 1 mov8; AFortMeshNetworkEventsLoader.h line 21). NodeType param unused.
// CONFIDENCE: HIGH
bMeshNetworkReady = true;
