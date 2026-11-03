# OnRep_InVehicle focused ABI evidence, batch 87

`PROPOSAL.md` documents two concrete argument recoveries and changes applied by root to
the existing `pawn_vehicle70_runtime/OnRep_InVehicle.rough.cpp`, SHA-256
`db071303aa5f56b5430acba8be362b602fc5d40218bc1d09859e6c2251d03187`.
The driver log must retain the returned AL byte and passes the enum-string data
or fallback based on its count. The old-driver montage virtual receives the same
pawn, montage, 1.0f, zero register arguments as the adjacent branch. Detailed
instruction bytes, branch evidence, source/image pins and ABI caveats are in
`PROPOSAL.md`. Root applied both corrections to the existing draft; no promotion is proposed. The SHA above identifies the pre-correction input.

The prior duplicate transcription was removed. Pinned improved decompilation
`porting/generated/decomp_improved/AFortPlayerPawn/OnRep_InVehicle.c` SHA-256
`e06c8febbbbd16927cfe045ce7ddfc8d74eaccf4bce755f1a7849d57a29d7cd8`.
Used `sha256sum`, bounded Python/Capstone reads of the existing runtime image and
existing draft, and source inspection. No Ghidra, build, generator, tests or
runtime run; image hash is inherited, not recomputed. The function remains
uncounted and ABI-incomplete. Root retains native/promotion/build ownership.
Preserve unrelated edits and configured bounded rotating fork-none workers.
