// Original body 0x14511a300 (thunk 0x14531e9c0). Offset evidence: Int32 +0x5c (both branches), Int32 +0x60 when byte +0x66 is nonzero else Int32 +0x58, bool +0x66 selects the branch. All four are unreflected gap members of this class (NativeRecovered members).
// The CMOVLE keeps the candidate when candidate+1 <= current+1, then INC returns the result plus one.
// CONFIDENCE: HIGH
const int32 CurrentLevel = NativeRecoveredInt32_0x5c;
const int32 CandidateLevel = NativeRecoveredBool_0x66 ? NativeRecoveredInt32_0x60 : NativeRecoveredInt32_0x58;
const int32 SelectedLevel = (CandidateLevel + 1 <= CurrentLevel + 1) ? CandidateLevel : CurrentLevel;
return SelectedLevel + 1;
