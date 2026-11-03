// CONFIDENCE: high; native capture and reflected fields reviewed.
{
	for (const FAshtonStoneState& Stone : StoneList)
	{
		if (Stone.StoneType == StoneType)
		{
			return Stone.StoneState;
		}
	}
	return EAshtonStoneStateType::MAX;
}
