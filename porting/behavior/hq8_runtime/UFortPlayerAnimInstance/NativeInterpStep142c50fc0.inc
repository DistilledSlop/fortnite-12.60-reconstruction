// Helper 0x142c50fc0 (UFortPlayerAnimInstance): float step from Current toward Target.
// Float ABI: XMM0 Current, XMM1 Target, XMM2 Rate, XMM3 DeltaTime.
// Constants read from .rdata: 0x1459c03a0 = 1.0e-8f (squared-distance epsilon), 0x1459a86f8 = 1.0f (step clamp).
// Callers store the result back to the same float (e.g. [RBX+0x7cc] in AFortPhysicsBall::UpdateDirectionAndRotation).
// CONFIDENCE: HIGH
if (!(DeltaTime > 0.0f))
{
    return Target;
}
const float Dist = Target - Current;
if (Dist * Dist < 9.99999993922529e-09f)
{
    return Target;
}
float Step = Rate * DeltaTime;
if (Step < 0.0f)
{
    Step = 0.0f;
}
else
{
    Step = (Step < 1.0f) ? Step : 1.0f;
}
return Current + Step * Dist;
