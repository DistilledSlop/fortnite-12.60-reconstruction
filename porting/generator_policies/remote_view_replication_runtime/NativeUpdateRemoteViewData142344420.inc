// CONFIDENCE: high
// Original 0x142344420; MULSS then CVTTSS2SI, including indefinite result.
if (!Controller) return;
const FRotator Rotation = Controller->GetControlRotation();
// Exact original bits 0x43360aab, not a double scale or UE angle compressor.
const __m128 Scale = _mm_set_ss(182.0416717529296875f);
const int32 Yaw = _mm_cvttss_si32(_mm_mul_ss(_mm_set_ss(Rotation.Yaw), Scale));
const __m128 Pitch = _mm_mul_ss(_mm_set_ss(Rotation.Pitch), Scale);
RemoteViewPitch = 0;
const int32 PitchInt = _mm_cvttss_si32(Pitch);
RemoteViewData32 = (static_cast<uint32>(PitchInt) & 0xffffu) << 16 |
    (static_cast<uint32>(Yaw) & 0xffffu);
