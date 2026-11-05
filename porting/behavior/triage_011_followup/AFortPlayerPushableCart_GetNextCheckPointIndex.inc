// CONFIDENCE: HIGH; direct captured wrapper.
{
	return static_cast<uint8>(FMath::Min(SplineCheckPointIndices.Num() - 1, static_cast<int32>(CurrentCheckPoint) + 1));
}
