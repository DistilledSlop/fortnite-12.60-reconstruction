// CONFIDENCE: MEDIUM. Exec thunk: reads the SkyTube object first (declaration order is reversed in the frame), then the out bool bIsFirstTube (CPF_OutParm), and forwards both to the native helper.
// Original address: 0x142aa5b70 (exec). Helper: 0x1426c86b0 (deps AFortProjectileBase__EnterSkyTube__Helper.json).
bool bIsFirstTube = false;
EnterSkyTubeHelper(SkyTube, bIsFirstTube);
// bIsFirstTube is written back to the script frame (out parameter).
