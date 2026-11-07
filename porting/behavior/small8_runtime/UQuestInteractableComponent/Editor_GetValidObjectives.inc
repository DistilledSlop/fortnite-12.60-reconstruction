// Original body address: 0x140693ec0
// Offset evidence: Body zeroes both qwords of the hidden TArray return slot (Data at +0, Num and Max at +8) and returns that slot, which is an empty TArray.
// CONFIDENCE: HIGH
return TArray<FString>();
