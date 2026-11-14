// CONFIDENCE: HIGH
// Original 0x141916010; full recovered body. CONFIDENCE: HIGH.
UPrimitiveComponent* Component = NativeRootPrimitive142323920();
const FVector Velocity = NativeVehicleVelocity1419193b0();
const FVector Forward = Component->GetForwardVector();
return (Forward.X * Velocity.X + Forward.Y * Velocity.Y + Forward.Z * Velocity.Z) * 0.036f;
