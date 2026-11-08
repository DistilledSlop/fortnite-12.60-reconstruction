// helper 0x142bfa260 (host UAmbientAudioStatics): KERNEL32 GetCurrentThreadId (import 0x1458fcaf8) compared with
//   (G1 != 0 ? G1 : G2) where G1=[0x1482472dc], G2=[0x1482472d0]
// evidence: engine IsInAudioThread() is exactly CurrentThreadId == (GAudioThreadId ? GAudioThreadId : GGameThreadId); the two globals are those engine thread ids
// CONFIDENCE: HIGH
return IsInAudioThread();
