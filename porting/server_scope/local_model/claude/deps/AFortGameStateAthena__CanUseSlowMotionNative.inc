// Native 0x14198bce0 (body of CanUseSlowMotionOnVictory). In CN mode the answer is this->Field1cb0 == 5. Otherwise every rule is asked through virtual slot 0x920 (rule decides with an out bool, default true); the first decisive rule's out value is returned, and true if no rule decides.
// Iterator: AFortGameStateAthena__CollectRules(nullptr, this).
if (IsCNModeActive())
{
	return Field1cb0 == 5;
}
FRuleIterator Rules = CollectRules(nullptr, this);
while (Rules.IsValid())
{
	bool bOut = true;
	bool bDecided = Rules.Current()->CanUseSlowMotion(bOut);
	if (bDecided)
	{
		return bOut;
	}
	Rules.Advance();
}
return true;
