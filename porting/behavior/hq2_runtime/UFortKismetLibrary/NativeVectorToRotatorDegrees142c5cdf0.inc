// Helper 0x142c5cdf0: FVector -> FRotator (Pitch, Yaw, Roll=0) in degrees. Callers use the returned pointer (RAX) as the rotator.
// Evidence: Yaw = Atan2Approx(Y, X)*K; Pitch = Atan2Approx(Z, sqrt(X*X+Y*Y))*K; Roll = 0; K = float at 0x145a179f4 = 57.2957763671875 (180/pi).
// Atan2Approx is the ported helper 0x142bbd380 (args: Y then X).
const float PlanarLength = FMath::Sqrt(InVector->X * InVector->X + InVector->Y * InVector->Y);
OutRotator->Yaw = UFortGameplayAbility::NativeAtan2Approx142bbd380(InVector->Y, InVector->X) * 57.2957763671875f;
OutRotator->Pitch = UFortGameplayAbility::NativeAtan2Approx142bbd380(InVector->Z, PlanarLength) * 57.2957763671875f;
OutRotator->Roll = 0.0f;
return OutRotator;
// CONFIDENCE: HIGH
