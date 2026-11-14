# Weapon fire FX cleanup and ability end

Complete original bodies `1426b60f0` (+690) and `142693270` (+758) are shared
across the 23 constructor-proven native weapon tables inventoried in
`generated/agents/weapon-dispatch187/constructor-dispatch.json`. World slot
+150 is AActor::GetWorld. The exact weak timer callback `14268e6ac` tail-calls
virtual +690; a virtual member-function delegate preserves that dispatch.

Inputs: `generated/native-cap-virtual4/0x1426b60f0.{c,asm}` and standard captures
`2f912fc32b104d449dd86833d589fdf7/0x142693270` and
`b6acb229fe7b457ea1d3ddcc4dd0b782/0x14268e6ac`. The two timer handles at original
3d0 and b18 are explicitly initialized invalid by constructor `14268d9b0`.
PersistentFXStartTime/MinimumTimeForPersistentFX and the two fire flags use actual
reflected storage. The original authority return precedes GetWorld. Minimum and
elapsed reads remain conditional; ordered positive minimum and unordered elapsed
match the original assembly. The first fallback sound is read after virtual
IsLocalController, preserving mutations during that call.

The three PrimaryFireStopSound entries, optional first-person sound, root-relative
location and four FireAudioChannels follow the captured order. Audio FadeOut uses
PrimaryFireSoundFadeOutTime and Linear=0. The event wrapper `142b44190` reads FName
global `1481c3f98`; initializer `1404533f0` uses literal `14656c268`, proving
OnStopWeaponFireFX. Timer clearing precedes handle invalidation and flag/event
updates. +758 directly invokes UGameplayAbility::K2_EndAbility when ActiveAbility
exists, then dispatches +690 unconditionally.

FortWeaponStopSoundNative.h is synthetic support for this exact call site of
engine audio helper `144224f50`, captured in
`generated/callee_probe/blocking/0x144224f50.c`. It preserves sound/engine/UseSound,
world resolution, audio permission and device gates plus every actual argument.
It does not add the dedicated-server gate found in this checkout's GameplayStatics
wrapper. It receives no original-address/body credit for the general helper.

Independent review caught sound-selection and conditional-read order before
integration. Engine audio ABI, generated delegates and runtime behavior remain
build/runtime pending; source completeness does not establish gameplay parity.
