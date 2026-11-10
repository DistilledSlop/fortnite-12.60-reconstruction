// Natives 0x141a49590 (HasFortGlobalsTagA, names at FFortGlobals +0x1748/+0x174c) and 0x141a4b0d0 (HasFortGlobalsTagB, +0x1768/+0x176c). Both check the object's two FName lists (+0xd0, count +0xd8 and +0xe0, count +0xe8). The name is taken from FFortGlobals; a None name never matches.
bool HasFortGlobalsTagA(const UObject* Obj)
{
	if (Obj == nullptr)
	{
		return false;
	}
	FName Name = GetFortGlobals()->NameA;
	if (Name.IsNone())
	{
		return false;
	}
	return Obj->NameListA.Contains(Name) || Obj->NameListB.Contains(Name);
}
