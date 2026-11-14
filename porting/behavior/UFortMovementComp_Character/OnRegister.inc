// Original address: 0x1420e7ad0
// CONFIDENCE: high
Super::OnRegister();

FallingSlopeSafeSlideAngleCached = FMath::Clamp(FallingSlopeSafeSlideAngleScalable.GetValueAtLevel(0.0f), 0.0f, 90.0f);
FallingSlopeSafeSlideNormalZ = FMath::Cos(FMath::DegreesToRadians(FallingSlopeSafeSlideAngleCached));
NativeViewAngleCos_65c = FMath::Cos(FMath::DegreesToRadians(NetworkSmoothingViewAngleThreshold));
NativeViewAngleSmallCos_664 = FMath::Cos(FMath::DegreesToRadians(NetworkSmoothingViewAngleThresholdSmall));
