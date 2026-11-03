# Round winner 80

One uncounted, unvalidated rough lift of `AFortMinigame::EvaluateRoundWinner`
(`0x14208c5c0`), with source hash in `sources.json`. It preserves the team
shortcut, per-bucket evaluation, record moves and reference counts, sorting,
selection, cleanup, and winner-array append. Calls, object layouts, stack inputs,
allocator behavior, atomic ordering and virtual dispatch ABIs require recovery.
The adapters for virtual calls with missing decompiler arguments intentionally
remain distinct from calls whose arguments are visible. Indeterminate stack
inputs represent original stack state, not initialized defaults. The two masked
stack words and record storage are reused across bucket iterations; the exact
stack inputs and sort/predicate ABI need native resolution.

The source was read using `cat porting/generated/native-broad80/0x14208c5c0.c`;
`sha256sum` produced the pinned digest. No compiler, tests, builds, generation,
native promotion, staging or commits were run for this lift. It is not parity
or build evidence.
