// Original address: 0x141d95970
// Evidence: EDX = NumOfPlayers. Zero -> ProgressReverseSpeed (+0x220) = 0.0f. Non-zero -> ProgressReverseSpeed = (float)CurrentCaptureSpeed (+0x224, int32). NumOfEmotingPlayers (R8) unused.
// CONFIDENCE: HIGH
if (NumOfPlayers == 0)
{
	ProgressReverseSpeed = 0.0f;
	return;
}
ProgressReverseSpeed = static_cast<float>(CurrentCaptureSpeed);
