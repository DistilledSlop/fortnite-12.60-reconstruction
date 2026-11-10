// CONFIDENCE: HIGH. Stores the battle-royale min and max draw distances and refreshes the cull distance.
// Original address: 0x142737f00 (exec). Native body: 0x141bc47c0 (stores min at +0x478, max at +0x47c, then virtual 0x8b0 = UpdateCullDistance).
BRMinDrawDistance = NewBRMinDrawDistance;
BRMaxDrawDistance = NewBRMaxDrawDistance;
UpdateCullDistance();
