// CONFIDENCE: HIGH
// Native 0x141efb390 uses the engine's scalar SinCos approximation.
float Sine, Cosine;
FMath::SinCos(&Sine, &Cosine, A * 0.017453292f);
return FVector(Cosine, Sine, 0.0f);
