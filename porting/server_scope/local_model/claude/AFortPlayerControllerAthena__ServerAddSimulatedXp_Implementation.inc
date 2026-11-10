// CONFIDENCE: LOW
// Original target 0x1419f0bc0. Quest manager lookup is vslot 0x1bd0, i.e. NativeGetQuestManager1423850c0(2).
// The only call made with that manager is FUN_1417256f0(manager, XpAmmount, &copy), which in this build
// only releases the shared reference held at its third argument (+8). It does not read the manager, XpAmmount
// or MetaData, so this build grants no XP; MetaData is unused.
if (auto QuestManager = NativeGetQuestManager1423850c0(2))
{
	FText SourceCopy = Source;	// copy adds a reference to the shared text data
	// FUN_1417256f0: releases SourceCopy's shared reference (no XP applied).
}
