// Helper 0x143fa3d30: MOV byte ptr [RCX+0x26],1 ; MAXSS XMM1,XMM0 (XMM0 = 0.0f) ; MOVSS [RCX+0x8],XMM1.
// MAXSS dest=XMM1 src=XMM0 yields (XMM1 > XMM0) ? XMM1 : XMM0, i.e. Value > 0 ? Value : 0.
// CONFIDENCE: HIGH - all 5 instructions accounted for; offsets are raw (no reflected member covers +0x8 / +0x26 by name).
uint8* VehicleBytes = reinterpret_cast<uint8*>(Vehicle);
*(VehicleBytes + 0x26) = 1;
*reinterpret_cast<float*>(VehicleBytes + 0x8) = (Value > 0.0f) ? Value : 0.0f;
