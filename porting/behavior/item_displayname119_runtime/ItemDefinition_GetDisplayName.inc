// CONFIDENCE: high
if (!bBaseName && !DisplayNamePrefix.IsEmpty())
{
    const FText Name = DisplayName;
    const FText Prefix = DisplayNamePrefix;
    return FText::Format(NSLOCTEXT("DisplayNameFormat", "DisplayNameFormatWithoutRarityPrefix", "{0} {1}"), Prefix, Name);
}
return DisplayName;
