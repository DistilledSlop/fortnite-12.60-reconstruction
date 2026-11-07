// Original body address: 0x14234efe0 (thunk 0x142a808a0)
// Offset evidence: CMP byte ptr [RCX+0x1065],DL -> reflected member StasisMode (Enum, EFortPawnStasisMode, 1 byte) at object +0x1065; receiver_delta 0.
// Callee: JMP 0x14232b1f0 -> AFortPlayerPawn::HandleInStasis() (tail call, void); JZ 0x14234eff3 -> RET when the value is unchanged.
// CONFIDENCE: HIGH
if (StasisMode != InStasisMode)
{
    StasisMode = InStasisMode;
    HandleInStasis();
}
