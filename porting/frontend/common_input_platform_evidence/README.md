# CommonInput platform cache preparation

Bounded original-image recovery; no runtime or full initialization parity claim.
`windows.json` pins four instruction windows, read with seek rather than loading
the image. The recorded whole-image hash is the previous baseline, not a new check.

`14081b550` obtains the settings object through `1407c4fc0`, invokes `14081d830`,
then copies seven platform bytes from settings+190, the soft-class array at+198
into result+8, and the resolved class array at+1a8 into result+18. Copies preserve
order and duplicates. Shipping reflection block145a0e320 identifies
`CurrentPlatform` at+190, `CommonInputPlatformData` at+50 and `InputDataClass` at+188.

`14081d830` is cache validation followed by initialization, not just a getter.
It ANDs byte+180 with validity of InputData's soft pointer at+28. It copies the
current soft-controller array, then for each entry ANDs the same byte with both
membership in the resolved class array (140818f70) and soft-pointer validity.
Membership compares class pointers, in order, and does not consume matches.
Nonempty unresolved soft paths invalidate the cache. The warning literal at
145a0db90 is `Trying to access unloaded CommmonInputSettings data. This may force a sync load.`
The spelling is original. It then tail-calls14081c1e0 regardless of that byte.

`14081c1e0` returns immediately when byte+180 is nonzero. Otherwise it invokes
1407d0ea0 on InputData, checks UClass and UCommonUIInputData inheritance, stores
InputDataClass+188, copies the platform bytes and soft-controller list from+50/+58,
copies the initial class array from+68, then iterates a snapshot of the soft list.
Each successfully resolved UCommonInputControllerData class is appended, without
deduplication. An object-index gate causes class root flags to be set in the
object array. The function finally sets byte+180 to1 even when some classes fail.
Keep these copies, repeated resolutions and rooting effects when implementing it.

Remaining: bind1407d0ea0 to the exact soft loading API and its diagnostics;
prove the object-index/rooting predicate and platform-data metadata/constructors;
close the module-name/factory registration described in
`../common_input_settings_evidence/HANDOFF.md`. A raw pointer-array copy or a
GetDefault substitution would omit these effects. No implementation is registered.
