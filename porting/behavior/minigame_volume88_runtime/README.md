# Volume minigame resolver

Complete typed source binding of native0x141c795a0. A null actor returns null;
otherwise call actor virtual+0x610 with UFortMinigameVolumeComponent class,
then return its CurrentMinigame pointer at+d0 or null when no component exists.
Do not add IsValid checks, manager lookup or current-state filtering.

The getter0x1429da540 names FortMinigameVolumeComponent and registers size0xd8.
Existing mutator_context_runtime/README.md proves the AFortVolume primary
vtable+0x610 ->0x143f709a0 first-owned-component class search, matching the
engine virtual FindComponentByClass. Existing canonical AppendMinigameOwner.inc
already binds this exact virtual and reflected CurrentMinigame+d0 field.
This helper reuses that proven contract without its caller's manager/state gates.

Root serial probes native88-volume and native88-close produced the ignored
C/assembly/metadata inputs pinned in source-pins.json. No new storage,
initialization, ownership or lifecycle is inferred. This is a complete body
proposal pending canonical generation; no build/runtime tests were run.

Root integrated these manifest proposals in generation88: all2,385 entries accepted, zero rejections. Bodies are canonical source bindings; no build or runtime validation since the2,040 baseline. Next tests3,040.
