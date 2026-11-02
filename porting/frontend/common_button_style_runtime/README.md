# CommonButton style reconstruction

Guarded native adapters for original helper1407acd30 and its direct refresh/size
operations. Exact class/property/optional-sound/event metadata and emitted member
types are checked before generation writes any output. SetStyle is not registered.

The implementation uses actual brush ownership, MID creation, legacy sound lookup,
three style caches, native virtual/BP event dispatch and fresh post-event size reads.
The weak internal target must exist before full rebuild; the original size operation
dereferences it without a null guard. Production initialization remains separate.

Original replays and operation mappings are retained in
/mnt/vibrato/fortnite-work/button-style-rebuild-dependencies-1/ and
button-style-rebuild-candidate-1/. Their complete-fragment and source checks pin the
supplied Win64 image338c83a2314ccac1040a2c0bb520b893013cc7b58eda58e550da67f5d84de680.
The candidate's 84 negative checks and the maintained policy tests validate guards;
engine runtime evidence is recorded separately in the project handoff.

The opt-in ButtonStyleRebuildProbe uses real objects and Slate. Its 93 rows include
64 sound combinations, legacy cache identity, material parent/outer, field mappings,
color shared-owner release, callback reentry and missing-CDO behavior. It observes
ProcessEvent dispatch, not a cooked Blueprint override. Private resource-handle
control-block ownership, nonempty-handle release, original pixels and audio playback
remain unverified; no full widget lifecycle or UI parity follows from this probe.
