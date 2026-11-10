// CONFIDENCE: LOW. Original 0x14292d330. The thunk reads the FText and passes a copy to 0x141eb0570, which calls the resolver 0x143f7a920 and releases the copy. Nothing is stored.
// Original address: 0x14292d330 (exec).
SetMissionRewardOverrideTag_Internal(MissionRewardOverrideTag);
