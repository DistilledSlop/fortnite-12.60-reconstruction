// Original address: 0x1420e5990
// CONFIDENCE: high
Super::Launch(LaunchVel);
if (LaunchVel.Z > 0.0f)
{
	FallingStartedZ = UpdatedComponent ? UpdatedComponent->GetComponentLocation().Z : FLT_MAX;
}
