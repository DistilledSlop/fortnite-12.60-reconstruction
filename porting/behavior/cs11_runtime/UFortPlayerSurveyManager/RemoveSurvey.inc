// Original body 0x142425f90 (thunk 0x142a952c0): SurveyQueue is the reflected TArray at +0x28 (Num at +0x30). Removes the first element equal to Survey (memmove tail, Num--).
// CONFIDENCE: HIGH
for (int32 QueueIndex = 0; QueueIndex < SurveyQueue.Num(); ++QueueIndex)
{
    if (SurveyQueue[QueueIndex] == Survey)
    {
        SurveyQueue.RemoveAt(QueueIndex);
        break;
    }
}
