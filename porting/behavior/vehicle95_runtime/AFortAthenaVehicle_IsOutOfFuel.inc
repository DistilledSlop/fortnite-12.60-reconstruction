// CONFIDENCE: high; recovered native body and reflected field mapping.
// 0x14283c0d0 -> 0x141b6a180; COMISS/SETBE also returns true for NaN.
return !(FMath::Abs(FuelCharge.Charge) > 1.0e-8f);
