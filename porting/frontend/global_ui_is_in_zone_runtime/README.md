`UFortGlobalUIContext::IsInZone() const` is a controller-type query, not a cached
zone flag. Original exec `14524a360` calls `144cd7a20`. That body casts the outer
to `ULocalPlayer`, obtains its controller for the virtual `GetWorld()` result,
and returns true for `AFortPlayerControllerZone`. Otherwise helper `144cb7e90`
with argument false repeats the outer/controller lookup and returns a controller
only if it is `AFortPlayerControllerSpectating`. Preserve the fresh lookup and
short-circuit behavior. Invalid outer/controller gives false.

The three class getters' inline names prove the types: `144786700` LocalPlayer,
`142a6ee20` FortPlayerControllerZone, `142a69680` FortPlayerControllerSpectating.
The original `14447f220` matches existing `UPlayer::GetPlayerController(const
UWorld*)` in Engine/Private/Player.cpp: null world uses stored controller;
otherwise iterate that world's controllers and select one owned by this local
player. Use the real Engine method and real outer/controller producers. No new
fields, registration shims, or fake world/controller are introduced.

`original-policy.json` pins reflection/native identities and bounded instruction
windows/type literals. `validate_original_image` uses pread; generation validates
metadata and final output. The runtime method is promoted through the generator,
never by hand-editing generated classes. This query does not complete gameplay
controller behavior or the broader frontend.
