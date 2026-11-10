// CONFIDENCE: HIGH. Original exec 0x1427daf90; native body 0x1426fe870 (inlined). Returns FFortRechargingActionTimer through ReturnValue.
// Returns BoostTimers[CurrentBoostTimerIndex] when the index is in range (BoostTimers at +0xf50, count at +0xf58, index at +0xfa0).
// Otherwise returns a function-local default built once through NativeInitBoostTimerDefaults141b5af50 with
// (0.1, 0.1, 0.1, 1.0, 1.0, 0, 0). The guard is the thread-safe local-static pattern in the original.
static FFortRechargingActionTimer DefaultBoostTimer = []()
{
    FFortRechargingActionTimer Default;
    AFortAthenaJackalVehicle::NativeInitBoostTimerDefaults141b5af50(reinterpret_cast<uint8*>(&Default), 0.1f, 0.1f, 0.1f, 1.0f, 1.0f, 0.0f, 0.0f);
    return Default;
}();

if (CurrentBoostTimerIndex >= 0 && CurrentBoostTimerIndex < BoostTimers.Num())
{
    return BoostTimers[CurrentBoostTimerIndex];
}

return DefaultBoostTimer;
