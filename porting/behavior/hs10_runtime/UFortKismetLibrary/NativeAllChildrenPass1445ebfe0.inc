// Helper 0x1445ebfe0 (10 callers; recursive, calls itself at 0x1445ec01 for each child).
// Asm: if (b[0x15]==0 && b[0x10]==0 && b[0x13]==0) return false; for each 0x10-byte element of array (Data +0x70, Num +0x78) with non-null first qword: if !Self(child) return false; return true.
// Offset evidence: Node has no recorded owning class; raw reads with asm widths (bytes at 0x10/0x13/0x15, qword at 0x70, int32 at 0x78).
// CONFIDENCE: HIGH
if (Node[0x15] == 0 && Node[0x10] == 0 && Node[0x13] == 0)
{
	return false;
}
const uint8* Elements = *reinterpret_cast<const uint8* const*>(Node + 0x70);
const int32 ElementCount = *reinterpret_cast<const int32*>(Node + 0x78);
for (int32 ElementIndex = 0; ElementIndex < ElementCount; ++ElementIndex)
{
	const uint8* Child = *reinterpret_cast<const uint8* const*>(Elements + static_cast<int64>(ElementIndex) * 0x10);
	if (Child != nullptr && !UFortKismetLibrary::NativeAllChildrenPass1445ebfe0(Child))
	{
		return false;
	}
}
return true;
