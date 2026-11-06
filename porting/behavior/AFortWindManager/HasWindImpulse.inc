// CONFIDENCE: HIGH
for (const FFortWindImpulseRadius& Impulse : WindImpulses)
{
	if (Impulse.Handle.UID == WindImpulseHandle.UID)
	{
		return true;
	}
}

for (const FFortWindImpulseCylinder& Impulse : WindImpulseCylinders)
{
	if (Impulse.Handle.UID == WindImpulseHandle.UID)
	{
		return true;
	}
}

return false;
