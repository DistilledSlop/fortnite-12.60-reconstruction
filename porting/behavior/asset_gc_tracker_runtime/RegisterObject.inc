// 0x141de9e00. Members are reconstructed names; this body requires the
// original tracker's shared lock, two UObject* sets and mode byte.
FScopeLock ScopeLock(&TrackerLock);
if (!bTemporaryMode)
{
    PersistentObjects.Add(Object);
}
else if (!PersistentObjects.Contains(Object))
{
    TemporaryObjects.Add(Object);
}
