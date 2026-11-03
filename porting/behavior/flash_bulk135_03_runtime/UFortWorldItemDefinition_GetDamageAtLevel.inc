// CONFIDENCE: high
// Original image bytes 0f57c0c3 at 1406964e0 disassemble xorps xmm0,xmm0; ret. Ghidra return-type inference was incorrect; Win64 float result is XMM0.
return 0.0f;
