// CONFIDENCE: MEDIUM
// Original address: 0x142a7aea0 (exec thunk). Captures: 0x1228 target captures/... native-cap-virtual3/0x142306ff0.c (132 lines, read in full) ; 0x1330 target native-cap-virtual/0x14233e6e0.c (185 lines, read in full).
// The STAT_Inv1PerfHook_PP_BS scope (FUN_142cd3770) is perf instrumentation and is omitted.
// bExcludeLaunchPadSkydive: (BalloonActiveCount[4] & 5) != 1. Byte offset 4327 bits: 0x01 = bLocalIsSkydiving, 0x04 = bIsSkydivingFromBus.
// So the flag is false only when the local pawn is skydiving and not from a bus.
const bool bExcludeLaunchPadSkydive = !(bLocalIsSkydiving && !bIsSkydivingFromBus);
// Slot 0x1228 (FUN_142306ff0) with (bFromBus = false, bExcludeLaunchPadSkydive). Name proposed.
ApplyParachuteOpenState(false, bExcludeLaunchPadSkydive);   // (bFromBus = false, bExcludeLaunchPadSkydive)
// Slot 0x1330 (FUN_14233e6e0) with bFromInput = false. Named ProcessParachuteOpen, name proposed.
ProcessParachuteOpen(false);
