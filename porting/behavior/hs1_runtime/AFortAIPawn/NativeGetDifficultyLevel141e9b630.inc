// helper 0x141e9b630 (host AFortAIPawn): MOVSS XMM0,[RCX+0xcc4]; RET
// evidence: single float load from Context+0xcc4 returned in XMM0; lookup_offset.py AFortAIPawn 0xcc4 finds no reflected member, so raw read
// CONFIDENCE: HIGH
return *reinterpret_cast<const float*>(reinterpret_cast<const uint8*>(Context) + 0xcc4);
