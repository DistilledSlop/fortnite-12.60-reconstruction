// Behaviour snippet: UFortHero::GetHeroTypeBP (body only; spliced into the generated member function).
// Original body address: 0x14070f440 (thunk 0x14293bc30)
// Offset evidence: MOV RAX,[this+0x88]; RET. +0x88 = inherited UFortPersistableItem::ItemDefinition (UFortPersistableItemDefinition*). Return type UFortHeroType* (class_ref_map). UFortHeroType derives from UFortPersistableItemDefinition, so static_cast is a valid downcast.
// CONFIDENCE: MEDIUM
return static_cast<UFortHeroType*>(ItemDefinition);
