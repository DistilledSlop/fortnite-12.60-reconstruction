// Native 0x1419b09b0 (body of ShouldHaveInfiniteResources). Asks every rule (virtual slot 0x9c8, with the pawn) and ORs the out bools. Stops early when a rule returns code 2.
// Iterator: AFortGameStateAthena__CollectRules(Pawn, this).
bool bResult = false;
FRuleIterator Rules = CollectRules(Pawn, this);
while (Rules.IsValid())
{
	bool bOut = false;
	uint8 Code = Rules.Current()->GrantsInfiniteResources(Pawn, bOut);
	bResult = bResult || bOut;
	if (Code == 2)
	{
		break;
	}
	Rules.Advance();
}
return bResult;
