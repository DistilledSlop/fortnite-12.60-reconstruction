# BlueprintContext runtime evidence tools

These tools verify the bounded generic GetContext implementation, not concrete
frontend subsystem behavior or the four product milestones. Only
`-ContextRuntimeProbe` enables the diagnostic observers and automatic driver.
Registry and lifecycle cases run in separate disposable client processes; the
lifecycle case uses normal OpenLevel travel and the game instance timer.

`source-expectations.json` pins the six manually maintained source inputs. The
v5 revision changes only the two lifecycle console-command object identifiers
to avoid unity-build shadowing. Console strings and handlers are unchanged.
Phase 1's failed v4 client build is retained in
`generated/material_pipeline/frontend-context-build-failure-checkpoint-results-1.json`;
its fresh UHT result does not imply a linked or runtime success.

The local source-evidence chain also requires the original generic candidate
receipt and patch under `/tmp/native-context-runtime-candidate-1/`, and the
independent root semantic audit under `porting/generated/frontend-native-evidence/`.
Their hashes are pinned in `provenance.py`. These original binary/evidence
inputs are deliberately outside Git. This is locally reviewable evidence,
not remote attestation or a hermetic build.

The sole coordinated Native build owner runs `provenance.py start`, both
`build` targets, then `finish`, using a new evidence directory for each phase.
Each invocation records its exact argv, log and exit status. Source inputs
must remain frozen throughout that phase. Fresh generated metadata, objects,
executables and debug files are required; unchanged exact response/unity
recipes can predate the phase. Preserve failures before releasing source.

After staging those exact binaries, use `provenance.py run` separately for
`--case registry` and `--case lifecycle`, with the real stage cwd, normal client
arguments and a 240-second timeout. Use `--help` for exact arguments. Environment
and Xvfb setup wrap the runner; it launches the binary directly and verifies
its hash and `/proc` executable identity. Do not force startup phases or add
an immediate Quit command. `verify_report.py` combines both-target original
metadata/object/linked ELF checks with strict actual reflection, registry,
GC, copied-map dispatch and travel observations. A zero process exit alone
does not establish success.

Run verifier regression tests from the repository root:

```sh
python3 -m unittest discover -s porting/frontend/context_runtime_probe/tests -v
```

The synthetic ELF, event mutations and portable source-contract checks test
the evidence tools; actual UE build and runtime acceptance remains separate.
`audit_manifest.py` replays the archived candidate's strict file manifest.


V6 loaded-property diagnostics preserve the exact raw original UHT/object/ELF audit.
Original ConstructFProperties class/object cases add exactly `0x0008001040000200`
to raw flags. The probe exposes the current typed `UObject*` traits computation,
requires that result exactly, and compares loaded flags with raw OR that exact
result. A static guard verifies all four enum-bit constants. No unknown-bit mask
is used. Three `CONTEXT_REFLECTION` rows measure found state, actual/expected
flags, offset, array dimension, field kind, PropertyClass and MetaClass. Missing,
duplicate, malformed or changed rows fail the independent measured-row verifier.
Raw metadata checks and generic runtime implementation are unchanged.

Use the existing `-stdout` launcher with the source-evidenced `-NOCONSOLE` option
for the two disposable client cases; keep other normal arguments, including
`-log`, unchanged. Unix normally installs a second console output device, so
without this transport option every observed event is printed twice. Never
normalize or deduplicate recorded events. Phase2 retains both original transports
and the actual loaded-property failures; V6 requires fresh measured live rows.
The original static constructor proof does not claim original runtime heap fields.
