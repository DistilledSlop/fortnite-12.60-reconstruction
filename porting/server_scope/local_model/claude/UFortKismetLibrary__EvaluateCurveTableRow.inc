// CONFIDENCE: HIGH
// Native 0x14296a330 -> 0x141eeca00 -> 0x144165480 (FCurveTableRowHandle::Eval): evaluates the handle's curve row at
// InXY into OutXY; returns false when the table or row is missing.
return CurveTableRowHandle.Eval(InXY, &OutXY, ContextString);
