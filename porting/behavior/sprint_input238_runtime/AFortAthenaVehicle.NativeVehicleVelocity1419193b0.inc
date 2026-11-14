// CONFIDENCE: HIGH
// Original 0x1419193b0; full recovered body. CONFIDENCE: HIGH.
UPrimitiveComponent* Component = NativeRootPrimitive142323920();
if (Component)
{
    FBodyInstance* Body = Component->GetBodyInstance(NAME_None, true);
    if (Body && Body->bSimulatePhysics)
    {
        return Component->GetComponentVelocity();
    }
}
return NativeRecoveredFVector_0xc40;
