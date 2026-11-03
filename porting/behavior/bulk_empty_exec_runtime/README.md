# Shared empty native exec recovery

Run from the repository root:

```sh
python3 -I porting/behavior/bulk_empty_exec_runtime/prepare.py
```

The script reads the current native function metadata, class name/parent maps,
class flags, selected native class set and behavior manifest. It verifies the
complete original mapped image SHA-256
`338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680`
(`/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin`,
base `0x140000000`). At `0x14075b6b0`, the exact 26 bytes are
`488b42204533c04885c0410f95c04c03c04c894220e9f64af2ff`.
They conditionally advance `FFrame.Code` and tail-jump to `0x1406801c0`,
whose first three bytes are `c20000` (`ret 0`). The generated C++ body is
`return;`; Unreal's generated exec handles the frame advancement.

Initial preparation: 391 matching methods; 367 candidates (359 FortniteGame, eight
FortniteUI), 24 rejected (15 already ported and nine outside target packages
and selected classes). Candidate original visibility: 81 public, 18 private,
268 protected; nine const, none static. Results depend on the current inputs.
The filter requires a unique mapped non-interface selected class in the target
packages, exact empty parameter metadata, known native flags, unambiguous
visibility, no network flags, Blueprint event, editor-only flag or existing
ported entry. Original numeric reflection flags, staticness, constness and
visibility are retained in each proposed entry.

Detailed candidates, rejected entries and reasons: `/tmp/gameplay-bulk-empty/report.json`.
Proposed entries: `/tmp/gameplay-bulk-empty/manifest-entries.json`. These are
preparation artifacts, not canonical manifest edits. All 367 candidates are now canonical and spliced; later default reruns therefore
exclude them as already ported. The combined client build passes. Server/runtime
validation is recorded in `../../generated/gameplay-bulk-integration-1/`. The evidence
establishes the original shared empty native body, not completion of any larger
feature or call path.
