// Behaviour snippet: UFortMusicCombatBank::GetAssetFromCombatIntensity (body only; spliced into the generated member function).
// Original address: 0x1424a9600 (thunk 0x1429fb880).
// asm: MOVSXD RAX,EDX; MOV RAX,[RCX+RAX*8+0x30]; RET. Assets is UFortMusicAsset* Assets[4] at +0x30 (reflected, array_dim 4).
// The asm has no bounds check on Intensity; an out-of-range index reads past the array, as the original does.
// CONFIDENCE: MEDIUM
return Assets[Intensity];
