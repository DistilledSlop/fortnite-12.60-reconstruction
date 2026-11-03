// CONFIDENCE: high; native capture and reflected fields reviewed.
{
	return FoodTeam == EBarrierFoodTeam::MAX ? FVector(1.0f, 1.0f, 1.0f) : FoodDisplayData[static_cast<uint8>(FoodTeam)].MeshScale;
}
