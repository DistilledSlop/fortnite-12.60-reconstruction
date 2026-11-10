// Native 0x14199e4e0, shared by IsLastManStandingCN (0x14291ffe0), IsStormCN (0x142920160 via thunk) and the slow-motion check (0x14198bce0).
// CONFIDENCE: MEDIUM. Reads the FortEngine singleton (0x141ed2350); returns false unless its flag at +0x2c8 is set. Mode is the object at this+0x1df8, falling back to this+0x1df0.
// Returns false if the mode's tag lists contain either of two FFortGlobals names (helpers AFortGameStateAthena__HasFortGlobalsTagInLists), or if the mode has a 0x467 mode byte not in {0,7}; otherwise true (including when no mode object exists).
if (!GetFortEngine()->bFortEngineFlag2C8)
{
	return false;
}
UObject* Mode = ModeObject1DF8 != nullptr ? ModeObject1DF8 : ModeObject1DF0;
if (HasFortGlobalsTagA(Mode) || HasFortGlobalsTagB(Mode))
{
	return false;
}
if (Mode != nullptr)
{
	FFortGlobals* Globals = GetFortGlobals();
	if (!Globals->NameC.IsNone())
	{
		if (Mode->TagsA.Contains(Globals->NameC) || Mode->TagsB.Contains(Globals->NameC))
		{
			return false;
		}
	}
	if (Mode->ModeByte467 != 0 && Mode->ModeByte467 != 7)
	{
		return false;
	}
}
return true;
