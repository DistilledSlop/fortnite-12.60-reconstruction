// CONFIDENCE: high; native reverse ExecuteIfSafe broadcast and deferred compaction.
bClientNotifiedOfLoss = true;
NativeLossNotification.Broadcast(static_cast<uint8>(LostReason));
