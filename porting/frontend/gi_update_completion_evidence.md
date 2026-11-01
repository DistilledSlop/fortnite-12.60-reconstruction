# GameInstance update-completion callback

The Init binding at the original UpdateManager+40 is
**UUpdateManager::OnUpdateCheckComplete**, whose argument is
**EUpdateCompletionStatus**. It binds GI member callback **141e70c30**.
This closes the enum/event identity left unresolved in the full Init proof.
The complete callback body and its account/player/service dependencies remain
to be ported; this evidence adds no runtime or milestone credit.

The independently captured original image is the same late image used by
`late_gi_init_control_flow_evidence.md`: SHA-256
`338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`,
MEMORY mapping with VA equal to140000000+fileoffset. No original process or
shared Ghidra project was accessed for this follow-up.

Original Init stores the constructed FortUpdateManager at GI+738, captures
141e70c30 and selects manager+40. The member callback accepts the byte in DL,
preserves/restores it across temporary register reuse, writes GI+979 and calls
**140fde930** with that byte. This helper is the original
`LexToString(EUpdateCompletionStatus)`: all eight jump-table cases and its
default yield the same nine strings as the pinned Engine implementation.
Original FEnumParams147881970, reached by singleton140fe3bf0, independently
supplies exactly the same enum name, scoped enumerator names and values0..8.
Value1 is UpdateSuccess. The separate EUpdateState enum has eight different
values and belongs to the other event.

An isolated control compiled the real Engine UpdateManager header, actual UHT
headers and module definitions with the UE Clang9 compiler. Static assertions
check the one-byte enum, success ordinal1, both24-byte native multicast types
and the exact completion getter return type. Generated assembly returns this+40
(decimal) for OnUpdateStatusChanged and this+64 for OnUpdateCheckComplete.
Thus hexadecimal+40 selects the completion event. This is actual Linux x86-64
compiler output, not an original Windows runtime layout measurement. The original
binding and matching enum/string producer independently corroborate the match.

The control uses a copied textual shared PCH, with no binary PCH or stale-line
aliases. It only emits LLVM/assembly into `/tmp/gi-update-callback-source-1`;
it does not run UHT, build/link UE, mutate Native/Engine or execute the callback.
Strace records1,468 successful regular read-only compiler opens. Frozen sources,
compiler, controls, command/results and outputs are indexed by the ignored
`generated/frontend-native-evidence/gi-update-completion-1/receipt.json`.
The compiler read closure is captured after compilation; this is local evidence,
not a hermetic or tamper-proof build. Exploratory disassembly was independently
replaced by the final standard-library byte replay.

Replay with `inspect_gi_update_completion.py --help`. It requires the pinned
late image, Engine header/source, control source and generated assembly, verifies
the original binding/argument anchors, enum singleton/data and all string cases,
then checks the compiled API offsets. Three regression tests reject swapped
event offsets, extra control instructions, changed inputs and wrong opcodes:

```sh
python3 -I porting/frontend/test_inspect_gi_update_completion.py
```

Future integration should use the existing typed
`UpdateManager->OnUpdateCheckComplete().AddUObject(...)` API with an
EUpdateCompletionStatus member parameter. Preserve the full source callback;
binding an empty handler or substituting EUpdateState would lose behavior.
