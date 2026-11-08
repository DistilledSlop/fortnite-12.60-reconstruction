// helper 0x140c08400 (host UFortAbilitySystemComponent): UTF-8 sequence decoder, no callers recorded
// evidence: RCX = pointer to a byte cursor (read at [RCX], written back at [R8]), EDX = bytes remaining (CMP EDX,n gates each length);
//   lead byte ranges 0x80/0xc0/0xe0/0xf0/0xf8/0xfc select 1..6 byte forms, continuation test (b & 0xc0) == 0x80,
//   result in EAX, 0x3f ('?') on any error; ranges 0x80..0x7ff, 0x800..0xfffd without surrogates, 0x10000..0x10ffff
// CONFIDENCE: MEDIUM
const uint8* SeqStart = *Cursor;
const uint32 LeadByte = SeqStart[0];
auto Reject = [Cursor](const uint8* NextPos) -> uint32
{
	*Cursor = NextPos;
	return 0x3fu;
};
auto IsCont = [](uint32 InByte) -> bool
{
	return (InByte & 0xc0u) == 0x80u;
};
if (LeadByte < 0x80u)
{
	*Cursor = SeqStart + 1;
	return LeadByte;
}
if (LeadByte < 0xc0u)
{
	return Reject(SeqStart + 1);
}
if (LeadByte < 0xe0u)
{
	if (Remaining < 2u)
	{
		return Reject(SeqStart + Remaining);
	}
	if (!IsCont(SeqStart[1]))
	{
		return Reject(SeqStart + 1);
	}
	const uint32 Cp2 = ((LeadByte - 0xc0u) << 6) | (SeqStart[1] - 0x80u);
	if ((Cp2 - 0x80u) > 0x77fu)
	{
		return Reject(SeqStart + 1);
	}
	*Cursor = SeqStart + 2;
	return Cp2;
}
if (LeadByte < 0xf0u)
{
	if (Remaining < 3u)
	{
		return Reject(SeqStart + Remaining);
	}
	if (!IsCont(SeqStart[1]) || !IsCont(SeqStart[2]))
	{
		return Reject(SeqStart + 1);
	}
	const uint32 Cp3 = (((LeadByte - 0xe0u) << 6 | (SeqStart[1] - 0x80u)) << 6) | (SeqStart[2] - 0x80u);
	if ((Cp3 - 0xd800u) <= 0x3ffu || (Cp3 - 0xdc00u) <= 0x3ffu || (Cp3 - 0x800u) > 0xf7fdu)
	{
		return Reject(SeqStart + 1);
	}
	*Cursor = SeqStart + 3;
	return Cp3;
}
if (LeadByte < 0xf8u)
{
	if (Remaining < 4u)
	{
		return Reject(SeqStart + Remaining);
	}
	if (!IsCont(SeqStart[1]) || !IsCont(SeqStart[2]) || !IsCont(SeqStart[3]))
	{
		return Reject(SeqStart + 1);
	}
	const uint32 Cp4 = ((((LeadByte - 0xf0u) << 6 | (SeqStart[1] - 0x80u)) << 6 | (SeqStart[2] - 0x80u)) << 6) | (SeqStart[3] - 0x80u);
	if ((Cp4 - 0x10000u) > 0xfffffu)
	{
		return Reject(SeqStart + 1);
	}
	*Cursor = SeqStart + 4;
	return Cp4;
}
if (LeadByte < 0xfcu)
{
	if (Remaining < 5u)
	{
		return Reject(SeqStart + Remaining);
	}
	if (!IsCont(SeqStart[1]) || !IsCont(SeqStart[2]) || !IsCont(SeqStart[3]) || !IsCont(SeqStart[4]))
	{
		return Reject(SeqStart + 1);
	}
	return Reject(SeqStart + 5);
}
if (Remaining < 6u)
{
	return Reject(SeqStart + Remaining);
}
if (!IsCont(SeqStart[1]) || !IsCont(SeqStart[2]) || !IsCont(SeqStart[3]) || !IsCont(SeqStart[4]) || !IsCont(SeqStart[5]))
{
	return Reject(SeqStart + 1);
}
return Reject(SeqStart + 6);
