// helper 0x142c97120 (host UCustomCharacterPartAnimInstance): RCX = hidden FString return, RDX = const FString& (Data@0, Num@8)
// evidence: first backward scan skips trailing '/' (0x2f) and '\\' (0x5c) to find the end, second backward scan finds the last separator for the start,
//   Start > End yields an empty string, otherwise FString::Mid(Start, End - Start) (clamps + substring copy through 0x142bc7780, ptr growth by 0x140680160/0x1406800e0)
// CONFIDENCE: MEDIUM
const int32 PathLen = InPath.Len();
int32 LeafEnd = PathLen;
while (LeafEnd > 0 && (InPath[LeafEnd - 1] == TEXT('/') || InPath[LeafEnd - 1] == TEXT('\\')))
{
	--LeafEnd;
}
int32 LeafStart = PathLen;
while (LeafStart > 0 && InPath[LeafStart - 1] != TEXT('/') && InPath[LeafStart - 1] != TEXT('\\'))
{
	--LeafStart;
}
if (LeafStart > LeafEnd)
{
	return FString();
}
return InPath.Mid(LeafStart, LeafEnd - LeafStart);
