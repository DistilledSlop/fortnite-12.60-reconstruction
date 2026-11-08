// Native helper 0x141f60cd0 (static helper on UFortMinigameObjectiveTrackerComponent; caller GetTrackedObjectivesByQuery body 0x142097140, two call sites).
// Asm: CMP CL,DL; equal returns 0 (XOR EAX,EAX); CL==0 gives 1; both non-zero gives 2 (TEST DL; JNZ ret); otherwise 1. No memory access or calls.
// CONFIDENCE: HIGH
if (First == Second)
{
	return 0;
}
if (First != 0 && Second != 0)
{
	return 2;
}
return 1;
