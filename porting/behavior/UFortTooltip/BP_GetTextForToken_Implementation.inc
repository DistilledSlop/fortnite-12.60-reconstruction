// CONFIDENCE: high
// Native 0x1425e4b40 formats the token into its localized bracketed token text.
OutText = FText::Format(NSLOCTEXT("Tooltip", "FormatString", "[{0}]"), FText::FromString(Token.ToString()));
return false;
