// Original 0x141bd99a0, exec 0x14273fc40; static disassembly. CONFIDENCE: HIGH
ULevel* ActorLevel = Actor ? Actor->GetLevel() : nullptr;
return ActorLevel && LevelStreamInfo && LevelStreamInfo->GetLoadedLevel() == ActorLevel;
