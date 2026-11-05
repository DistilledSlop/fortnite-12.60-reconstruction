// Behaviour snippet: UFortAccoladeItemDefinition::GetPreviousAccoladeLargePreviewImage (body only; spliced into the generated member function).
// Original address: 0x14278e580
// Source: local-model fn runner; static checks only, not compiled or runtime verified.
// CONFIDENCE: HIGH
if (!PreviouseAccoladeLargePreviewImage.IsNull())
{
	return PreviouseAccoladeLargePreviewImage;
}
return PreviouseAccoladeSmallPreviewImage;
