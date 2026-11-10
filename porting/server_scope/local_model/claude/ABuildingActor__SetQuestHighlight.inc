// CONFIDENCE: MEDIUM. Toggles the quest highlight reason in the building's highlight-reason bitmask and refreshes the highlight. The highlight is marked dirty only when the quest reason is or becomes the top-priority reason.
// Original address: 0x1427384e0 (exec). Native body: 0x141bc4dd0. Bitmask at +0x3ee (unnamed); bit value 0x08 is the quest reason. Bit 0x04 of +0x3e9 is bHighlightDirty.
const uint8 QuestHighlightBit = 0x08;
const uint8 HigherPriorityMask = QuestHighlightBit - 1;

if (bEnabled)
{
    if ((HighlightReasonMask & QuestHighlightBit) == 0)
    {
        if ((HighlightReasonMask & HigherPriorityMask) == 0)
        {
            bHighlightDirty = true;
        }
        HighlightReasonMask |= QuestHighlightBit;
        HighlightFlagsB |= QuestHighlightBit;
    }
}
else if ((HighlightReasonMask & QuestHighlightBit) != 0)
{
    if ((HighlightReasonMask & HigherPriorityMask) == 0)
    {
        bHighlightDirty = true;
    }
    HighlightReasonMask &= ~QuestHighlightBit;
}

UpdateHighlightState();
