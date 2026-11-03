# Creative RPC validators

Seven original validators map directly to current declarations and fields.
Destroy, duplicate, restore and rename check the first IslandId FString stored
signed character count >1, including the terminator. They share native folded
body140f6b550 with two validators in controller_integration82_runtime.
MakeNewCreativePlot checks its first PlotType pointer for nonnull (14097cd20).
The load/save checkpoint validator1419f1db0 checks a nonnull Portal equals
controller OwnedPortal, reflected object property offset13608 (0x3528).
No extra string-content validation, defaults or object validity tests are added.

Signatures follow current reflected generated declarations, not reversed/missing
old manifest guesses. Existing generator validate support adds WithValidation
and matching declarations; root generation is required. sources.json pins the
original generated decompile sources. No build, runtime or tests were run.
