// Original body: 0x1426c8b20 (thunk 0x142aa5c40)
// Offset evidence: MOV byte [R8],0; if RDX!=0 and [RCX+0x7a8]==RDX: [R8]=1, [RCX+0x7a8]=0
// CONFIDENCE: HIGH
bIsLastTube = false;
if (SkyTube != nullptr && CurrentSkyTube == SkyTube)
{
    bIsLastTube = true;
    CurrentSkyTube = nullptr;
}
