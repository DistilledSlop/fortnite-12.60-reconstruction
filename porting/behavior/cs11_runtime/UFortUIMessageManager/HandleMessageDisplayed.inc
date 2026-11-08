// Original body 0x144dc0e00 (thunk 0x145314d90): MessageQueue (reflected TArray at +0x38, Num +0x40) drops every entry equal to MessageItem with order kept (run-compaction loop with memmove, Num set to kept count), then a weak pointer to MessageItem is appended to CurrentlyDisplayedMessages (+0x48, Num +0x50).
// CONFIDENCE: MEDIUM
MessageQueue.Remove(MessageItem);
TWeakObjectPtr<UFortUIMessageItemWidget> DisplayedWeakItem = MessageItem;
CurrentlyDisplayedMessages.Add(DisplayedWeakItem);
