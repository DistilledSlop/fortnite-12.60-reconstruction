// Original address: 0x140693ec0
// Evidence: hidden return buffer in RDX; body writes two zero qwords ([RDX], [RDX+8]) and returns the buffer. That is a value-initialized TArray<FString> (Data=0, Num=0, Max=0).
// CONFIDENCE: MEDIUM
return TArray<FString>();
