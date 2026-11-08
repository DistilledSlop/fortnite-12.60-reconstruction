// Helper 0x142d30420. Scans the FString (Data at +0, Num at +0x8, signed) for wide char L'/' (0x2f).
// Evidence: Num==0 -> not found; otherwise a found hit returns (index == -1) which is false; not found returns true.
// Caller 0x142ed4880 (`CALL 0x142d30420; TEST AL,AL; JNZ`) uses the result as "path has no slash".
// CONFIDENCE: HIGH
const int32 Count = Path.Len();
for (int32 Index = 0; Index < Count; ++Index)
{
	if (Path[Index] == TEXT('/'))
	{
		return false;
	}
}
return true;
