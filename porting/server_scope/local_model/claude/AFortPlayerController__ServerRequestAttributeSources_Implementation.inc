// CONFIDENCE: LOW. Impl at 0x1423abf70 is fully read. FUN_1415fa070, FUN_1406aade0 and FUN_142a3c410 are not read.
if (AbilitySystemComponent != nullptr)
{
    FAttributeSourceList Sources;
    GatherAttributeSources(AbilitySystemComponent, Attribute, Sources);   // FUN_1415fa070
    ReplyAttributeSources(this, GetReplyContext(Attribute), AbilitySystemComponent, Sources);   // FUN_1406aade0 + FUN_142a3c410
    // Sources' per-entry buffers are released here (loop over 0x20-byte entries, buffer at +0x10).
}
// Attribute's two arrays are released on every path.
