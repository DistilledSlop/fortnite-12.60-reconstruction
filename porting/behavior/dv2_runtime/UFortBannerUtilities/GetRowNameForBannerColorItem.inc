// Original body address: 0x141ed7610 (thunk 0x14284adf0, static)
// Offset evidence: BannerColor +0xe8 = ColorRowName (FName, reflected public member of UFortHomebaseBannerColorItem); a null argument returns the zero FName (NAME_None).
// CONFIDENCE: HIGH
static_assert(sizeof(UFortHomebaseBannerColorItem) > 0, "UFortHomebaseBannerColorItem must be complete");
if (!BannerColor)
{
	return FName();
}
return BannerColor->ColorRowName;
