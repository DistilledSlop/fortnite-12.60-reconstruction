# Time-of-day theme selection

`FortTimeOfDayThemeSelection.h` proposes source bodies for 0x141f4fed0,
0x141f5ba50 and 0x141f60280. It is not integrated. They use the existing
reflected theme and collection objects and preserve owning soft-pointer copies,
ordered appends (including duplicates), path-based prohibited matches, swap
removal and its changed ordering, and the lazy rebuild only when the valid
cache is empty. The selected soft class is copied by value. The original
selection uses the platform `rand`-based `FMath::RandHelper` calculation;
the Windows build should use the matching platform implementation. A still
empty cache returns an empty soft class, and is rebuilt again on the next call.

Evidence: original probes `/tmp/tod-probe/0x141f60280.*`,
`/tmp/tod-probe2/0x141f5ba50.*`, and `/tmp/tod-theme-probe/0x141f4fed0.*`;
the latter was probed from the same read-only Ghidra project with transaction
rolled back. The equality helper compares the soft object path at offset 0x10;
`RemoveAtSwap(Index, 1, true)` matches the observed last-element swap and
shrink call. No tests or builds were run, per task constraint.

The three .inc helpers are now integrated on UFortTimeOfDayData; the original
header is retained as proposal provenance and is not emitted.
