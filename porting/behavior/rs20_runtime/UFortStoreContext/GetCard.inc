// Original body address: 0x144d01620 (thunk 0x1452f1c80). Offset evidence: CardList is the reflected TArray<FCard> at object +0x80 (Num at +0x88); element stride 0x18 equals sizeof(FCard) (16-byte copy at element +0, 8-byte copy at +0x10). Valid when 0 <= CardIdx < Num.
// CONFIDENCE: HIGH
if (CardIdx < 0 || CardIdx >= CardList.Num())
{
	return false;
}
ResultOut = CardList[CardIdx];
return true;
