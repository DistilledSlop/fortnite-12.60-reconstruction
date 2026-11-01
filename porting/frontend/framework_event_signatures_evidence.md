# Remaining framework event registrations

The 13 remaining non-Native events in the fixed 41-event frontend inventory
match the original signatures in both compiled targets. This closes that
registration inventory only. Native lifecycle behavior, actual widgets,
interactive frontend acceptance and the four product milestones remain open.

The new cases are `UCommonButton::BP_OnHovered` and these `UUserWidget` events:
`Construct`, `Destruct`, `OnAnalogValueChanged`, `OnInitialized`,
`OnMouseButtonDown`, `OnMouseButtonUp`, `OnMouseEnter`, `OnMouseLeave`,
`OnTouchEnded`, `OnTouchStarted`, `PreConstruct` and `Tick`.

## Evidence

The independently produced report is
`generated/frontend-framework-signatures-2/root-framework-elf-signature-audit-3.json`,
SHA-256 `6fb0f2fbba83e05a9b3933ff8aabc898eeab673e06bcd2fb7ca762c538d821fe`.
It records 26 passing client/server comparisons, all original raw windows and
an immutable copy of the inspector. The earlier audit remains preserved;
audit 2 additionally verifies compiled string literals and original constructor
references to the exact function records. Audit 3 binds those unchanged
comparisons to the independently verified source/binary checkpoint, addressing
the provenance gap identified by a separate read-only review.

Original bytes come from the read-only memory image
`generated/dump/FortniteClient-Win64-Shipping.runtime.bin`, SHA-256
`604900985a724144b6e0fc6e033b0cef2adb74a295a65a4fe01aceebb52df93d`.
Its layout is `VA = 0x140000000 + file offset`. For each event, the inspector
finds the unique `FFunctionParams` matching the original owning-class
constructor and function name, and checks that the original function
constructor contains `LEA RDX,[RIP+disp32]` targeting that exact record.
The 128-byte constructor windows are bounded evidence, not function extents.

The comparison reads actual ELF metadata rather than accepting declarations
or symbol presence. It checks the function name, owner constructor, parameter
structure size, function/object flags, serialized property order, property
name/flags/kind/dimension, parameter offsets, bool setter offsets and exact
referenced struct constructors. Generated source contains the Blueprint
`ProcessEvent` forwarder and no Native `exec` stub for each event.

`generated/material_pipeline/media-social-registration-signatures-1.json`
binds 16 frozen generated-source, ELF-object, response and unity-source files
under `/mnt/vibrato/fortnite-evidence/frozen-runtime/media-social-frozen-runtime-1/registration-signatures/`.
The compiled objects are `Module.CommonUI.gen.cpp.o` and
`Module.UMG.gen.7_of_11.cpp.o` for both `FortniteLinux` and
`FortniteLinuxServer`. Both builds completed successfully at native source
`2f40d73` and Engine `d7e04d4b6`; the report hashes their result files and logs.
The failed-phase checkpoint
`generated/material_pipeline/frontend-media-social-failure-checkpoint-results-1.json`
has SHA-256 `147b551097d98e73380586ebd660a51ac054e3df5b6a97cb06a8bffa58842536`.
Its independent integrity audit verifies 92 committed source blobs/snapshots,
18 binary records/six immutable binaries, 16 signature snapshots and 689
artifact hashes, while explicitly retaining `phase_passed: false`.

## Limits

This paired build's first Social material probe failed its atlas metadata
gate before rendering diagnostics. That failure does not become a material
pass through this signature audit. No Social/Media acceptance, new join,
widget behavior, animation playback or original pixel parity is inferred.
The two Native panel intro/outro targets remain excluded from this bounded
non-Native registration count and still require their real behavior.
