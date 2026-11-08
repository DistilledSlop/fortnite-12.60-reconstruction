// Behaviour snippet: AFortCreativeMoveTool::ServerUpdatePrecisionMode_Implementation (body 0x1426819c0, thunk 0x1428c5150).
// Offset evidence: byte store of the bPrecisionMode argument (DL) at object +0x1128 (this_delta 0). +0x1128 falls inside reflected struct WantsGravityOnPlace (+0x111c) at a non-member offset, so raw access.
// Asm: store DL to [RCX+0x1128]; MOVZX EDX,R8B; JMP 0x142681e90 (NativeClampGridSnapIndex142681e90, tail call with RCX = this).
// CONFIDENCE: HIGH
uint8* Raw = reinterpret_cast<uint8*>(this);
*reinterpret_cast<uint8*>(Raw + 0x1128) = static_cast<uint8>(bPrecisionMode);
AFortCreativeMoveTool::NativeClampGridSnapIndex142681e90(this, PrecisionModeIndex);
