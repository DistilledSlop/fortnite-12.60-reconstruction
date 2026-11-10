// CONFIDENCE: MEDIUM
// Original address: 0x142a667a0 (exec thunk). Validate slot +0x1f48 -> 0x1406801d0 (STUB: return true), so the
// RPC_ValidateFailed branch is unreachable. Implementation slot +0x1f50 -> 0x1423e8a30 (dep file).
// Parameter order follows the thunk read order: the first read is InPoints, the second is InAirTimeX1000.
// Reflection lists InAirTimeX1000 first; the thunk order is the reverse, so the call uses (InPoints, InAirTimeX1000).
ServerAwardVehicleTrickPoints_Implementation(InPoints, InAirTimeX1000);
