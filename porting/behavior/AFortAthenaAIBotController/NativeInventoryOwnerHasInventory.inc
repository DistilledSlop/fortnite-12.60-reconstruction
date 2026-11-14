// CONFIDENCE: HIGH
// Original 0x1416a0ae0. Concrete body; shared interface binding is tracked separately.
AFortInventory* ItemInventory = Inventory;
// Identical body: AFortAIPawn 0x1422dcd90 and AFortAthenaAIBotController 0x1416a0ae0.
return Inventory && ItemInventory->HasActorBegunPlay();
