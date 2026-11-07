// Behaviour snippet: AFortHoagieAudioController::CacheAudioComponents (body only; spliced into the generated function).
// Original body address: 0x14084b600 (thunk 0x1408592f0). Offset evidence: MOV [RCX+0x230],RDX = EngineAudio (reflected Object at +0x230); MOV [RCX+0x238],R8 = RotorAudio (reflected Object at +0x238).
// The generated signature is (InEngine, InRotor): RDX is the first parameter InEngine and R8 is the second parameter InRotor, so the names match the members.
// CONFIDENCE: HIGH
EngineAudio = InEngine;
RotorAudio = InRotor;
