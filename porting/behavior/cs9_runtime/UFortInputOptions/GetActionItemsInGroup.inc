// Original body 0x144e71950 (thunk 0x14525f520). Offset evidence: this+0x310 is the InputData TArray; element byte at UFortOptionsMenuInputData+0x98 is ActionKeyMapping.ActionGroup (struct at 0x90, ActionGroup at +8). Loop appends entries whose group equals the argument.
// CONFIDENCE: HIGH
TArray<UFortOptionsMenuInputData*> Result;
for (UFortOptionsMenuInputData* InputEntry : InputData)
{
    if (InputEntry->ActionKeyMapping.ActionGroup == InputActionGroup)
    {
        Result.Add(InputEntry);
    }
}
return Result;
