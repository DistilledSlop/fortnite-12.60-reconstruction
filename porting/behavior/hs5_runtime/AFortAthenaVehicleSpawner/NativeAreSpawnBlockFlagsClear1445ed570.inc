// Native 0x1445ed570: returns true only when the byte flags at +0x12 and +0x11 are both zero.
// Offset evidence: CMP byte [RCX+0x12],0 / JNZ false; CMP byte [RCX+0x11],0 / JNZ false; MOV AL,1; RET.
// No reflected member covers +0x11/+0x12 (lookup_offset reports the first member at 0x218), so raw byte reads.
// CONFIDENCE: HIGH
const uint8* SpawnerBytes = reinterpret_cast<const uint8*>(Spawner);
if (SpawnerBytes[0x12] != 0)
{
    return false;
}
if (SpawnerBytes[0x11] != 0)
{
    return false;
}
return true;
