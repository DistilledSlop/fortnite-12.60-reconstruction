// Original body address: 0x14200b810 (thunk 0x14292f150); callee 0x142930ca0 = UFortGiftBoxItemDefinition::StaticClass().
// Offset evidence: this+0x88 = UFortPersistableItem::ItemDefinition (reflected Object in generated ancestor). The asm returns the pointer only when it passes a class-depth/ancestor IsA test against that static class, else null.
// CONFIDENCE: MEDIUM
return Cast<UFortGiftBoxItemDefinition>(ItemDefinition);
