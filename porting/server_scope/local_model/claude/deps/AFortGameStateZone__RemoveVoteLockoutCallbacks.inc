// Native helper 0x1406a9dc0 (called by NotifyEndFailedGameplayVoteLockout). Removes matching entries from a TArray and returns them. Semantics only: RemoveAll(key). Not ported line by line.
TArray<FVoteLockoutCallback> Removed = Entries.RemoveAll(Key);
return Removed;
