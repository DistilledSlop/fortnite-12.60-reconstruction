// Original body: 0x1423ac470 (thunk 0x142a4d160). Offset evidence: [RCX+0x1730] is a dword RMW of bit 0x1 = AFortPlayerController::bAutoEquipBetterItems (declared bitfield, porting/generated/reflection/bool_bits.json); the bit is set from the bAutoEquip argument (DL).
// CONFIDENCE: HIGH
bAutoEquipBetterItems = bAutoEquip;
