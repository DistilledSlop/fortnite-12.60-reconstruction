// Original address: 0x141b9c710.
// CONFIDENCE: MEDIUM
// Virtual slot 0x8b8, returns FVector through hidden pointer; see building_vitals_runtime/README.md.
const FQuat Rotation = (RootComponent != nullptr) ? RootComponent->GetComponentTransform().GetRotation() : FQuat::Identity;
const FVector Location = (RootComponent != nullptr) ? RootComponent->GetComponentTransform().GetTranslation() : FVector::ZeroVector;
return Rotation.RotateVector(CentroidOffset) + Location;
