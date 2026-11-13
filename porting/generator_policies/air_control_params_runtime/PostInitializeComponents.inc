// Original AFortSkyTube::PostInitializeComponents 0x1427057c0; CONFIDENCE: HIGH.
Super::PostInitializeComponents();
constexpr float NativeRadiansPerDegree = 0.01745329238474369f; // 0x3c8efa35
NativeEntryVelocityThreshold_0328 = FMath::Cos(EntryVelocityAngleRequired.GetValueAtLevel(0.0f, nullptr) * NativeRadiansPerDegree);
NativeSteeringMagnetThreshold_0538 = FMath::Cos(SteeringMagnetMaxSteerAngle.GetValueAtLevel(0.0f, nullptr) * NativeRadiansPerDegree);
NativeActiveForwardSteeringThreshold_0350 = FMath::Cos(ActiveForwardSteeringAngle.GetValueAtLevel(0.0f, nullptr) * NativeRadiansPerDegree);
