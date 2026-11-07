// Behaviour snippet: UFortBannerUtilities::GetRowNameForBannerColorItem (body only; spliced into the generated function).
// Original body address: 0x141ed7610. Offset evidence: BannerColor (RDX) null -> zero FName written to the hidden return slot; otherwise MOV RAX,[RDX+0xe8] = ColorRowName (Name, reflected at +0xe8 on UFortHomebaseBannerColorItem).
// CONFIDENCE: HIGH
if (!BannerColor) { return FName(); }
return BannerColor->ColorRowName;
