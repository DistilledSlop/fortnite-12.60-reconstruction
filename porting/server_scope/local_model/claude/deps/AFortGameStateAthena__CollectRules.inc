// Iterator family 0x1419b90a0 (init), 0x1419f3d90 (advance), 0x1419bb7e0 (close). CONFIDENCE: LOW. Collects rule components from two owners (first arg may be null, second is the game state). Each owner that has a rule component contributes it, otherwise its class is searched. Duplicates are skipped; advance skips entries whose weak pointer is gone.
FRuleIterator Iterator;
Iterator.Reset();
if (FirstOwner != nullptr)
{
	UObject* Component = FirstOwner->FindRuleComponent();
	if (Component != nullptr)
	{
		Component->GatherRules(Iterator);
	}
	else
	{
		Iterator.AddUniqueFromClass(FirstOwner);
	}
}
if (SecondOwner != nullptr)
{
	Iterator.AddUnique(SecondOwner);
}
Iterator.SeekFirstValid();
return Iterator;
