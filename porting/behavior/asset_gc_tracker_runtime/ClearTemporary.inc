// 0x141dfc1e0. TemporaryEntries is the original +0x150 container; its
// element type and other producer must be recovered before integration.
FScopeLock ScopeLock(&TrackerLock);
TemporaryObjects.Empty();
TemporaryEntries.Empty();
