# Kismet geometry batch

Four reflected static functions have complete candidate bodies and pinned exec/helper windows. GetAngleDegrees takes const-reference vectors; the other vector inputs are by value. The reflection flags are 0x4c22403 for GetAngleDegrees and 0x14822401 for the other three; none has an out parameter. Struct 0x142eea590 is FVector. Helper addresses are in the manifest; wrappers and underlying math helpers are pinned in native-evidence.json.

GetAngleDegrees tests squared length against 1, then ordered >= 1e-8; the initial unordered comparison retains a vector with NaN squared length. It uses two-pass SSE inverse square root, dot clamp, acos and degrees. Closest-axis starts at +X, tests dot converted to double > 0.7071 and rotates four times with the original accumulated quarter-turn matrix (whose off-diagonal coefficient rounds to 1.0f); if none qualifies it returns the original vector. SinCos and Atan2/NormalizeAxis match the corresponding Unreal math code; the latter maps to (-180, 180]. These ports rely on the platform math implementation matching the original x86 UE implementation, including FMath::InvSqrt, SinCos, Atan2 and Fmod. SinCos's integer conversion outside its usual domain has platform-dependent C++ behavior. No build or runtime tests were performed. Next testing 3040; source 2048.

Integrated into the manually owned FortniteMapClasses header/source. These four
reflected bodies are outside canonical generated-class splice accounting (which
remains 2,051 bodies +176 fields). Their declarations, source bodies, complete
reflection records and original byte windows are checked during generation by
source_policy.py. Promotion65 accepted2,227/rejected0 and changed no generated
files. This is source evidence only; no build/runtime/tests since baseline2,040.
