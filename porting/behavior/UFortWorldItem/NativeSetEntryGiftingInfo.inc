// CONFIDENCE: HIGH
NativeCopyGiftingInfo(Entry.GiftingInfo, Info);
Entry.bWasGifted = Entry.GiftingInfo.HeroType != nullptr && Entry.GiftingInfo.PlayerName.GetCharArray().Num() >= 2;
NativeMarkEntryDirty(Entry);
