// Concrete producer of router native gate99. The selected root operation is real
// 1407f4510, requiring its shared-root type, deactivation and binding lifecycle.
void UCommonUIActionRouter::NativeSetRootGate1407f78c0(bool bEnabled)
{
	NativeGate99 = bEnabled;
	if (!NativeGate98 && !bEnabled)
	{
		NativeSetSelectedRoot1407f4510({});
	}
}
