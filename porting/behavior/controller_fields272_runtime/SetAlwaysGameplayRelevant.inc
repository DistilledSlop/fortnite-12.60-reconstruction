// CONFIDENCE: high
// Preserve the other flags while replacing bit 2 and resetting the adjacent word.
GameplayRelevanceFlags &= ~uint32(4);
GameplayRelevanceState = 0xbf800000u;
GameplayRelevanceFlags |= bInAlwaysGameplayRelevant ? uint32(4) : uint32(0);
