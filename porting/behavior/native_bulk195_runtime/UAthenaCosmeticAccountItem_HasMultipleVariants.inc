// Native body 0x141d53280. CONFIDENCE: high
for (const FMcpVariantChannelInfo& Channel : VariantChannels)
{
    if (Channel.OwnedVariantTags.Num() > 1) return true;
}
return false;
