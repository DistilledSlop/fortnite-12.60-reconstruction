// CONFIDENCE: HIGH
// Original address: 0x142a1ef80 (exec thunk), native helper 0x142341760 (dep AFortPawn__PlayLocalAnimMontageImpl)
// Script read order: NewAnimMontage (object), InPlayRate (float), StartSectionName (FName), bNoBlend (bool).
// Return value is the montage length returned by the anim-instance play call (0 when nothing played).
ReturnValue = PlayLocalAnimMontageImpl(NewAnimMontage, InPlayRate, StartSectionName, bNoBlend);
