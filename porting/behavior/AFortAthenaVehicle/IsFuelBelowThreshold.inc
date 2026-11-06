// CONFIDENCE: high; recovered native body and reflected field mapping.
// 0x14283bf40 -> 0x141b6a180: Charge at FuelCharge+0xc4; threshold at +0xc0.
// COMISS/SETC treats unordered comparisons as true.
return !(FuelCharge.Charge >= FuelCharge.ChargeThreshold);
