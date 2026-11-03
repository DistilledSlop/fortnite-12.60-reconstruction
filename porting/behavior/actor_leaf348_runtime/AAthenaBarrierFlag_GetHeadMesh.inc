// CONFIDENCE: high; native capture and reflected fields reviewed.
{
	return FoodTeam == EBarrierFoodTeam::MAX ? nullptr : FoodDisplayData[static_cast<uint8>(FoodTeam)].HeadMesh;
}
