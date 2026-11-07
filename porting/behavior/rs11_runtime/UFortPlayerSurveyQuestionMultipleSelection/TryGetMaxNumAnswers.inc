// Behaviour snippet: UFortPlayerSurveyQuestionMultipleSelection::TryGetMaxNumAnswers (body only).
// Original address: 0x14242b7f0
// Offset evidence: this+0x58 = MinNumAnswers (reflected Int); this+0x5c = MaxNumAnswers (reflected Int).
// Succeeds when MaxNumAnswers > 0 and MaxNumAnswers >= MinNumAnswers; otherwise writes -1 and fails.
// CONFIDENCE: HIGH
if (MaxNumAnswers > 0 && MaxNumAnswers >= MinNumAnswers)
{
    OutMaxNumAnswers = MaxNumAnswers;
    return true;
}
OutMaxNumAnswers = -1;
return false;
