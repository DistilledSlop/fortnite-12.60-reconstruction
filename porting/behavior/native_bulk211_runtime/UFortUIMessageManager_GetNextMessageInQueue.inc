// CONFIDENCE: high; native callee reads the first entry of the reflected MessageQueue.
return MessageQueue.Num() > 0 ? MessageQueue[0] : nullptr;
