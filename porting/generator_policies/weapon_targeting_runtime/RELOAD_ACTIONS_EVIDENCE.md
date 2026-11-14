# Reload and ranged targeting callers

Complete originals: AFortWeapon `1426aecd0` (+838) and `1426b7a40` (+848), and
AFortWeaponRanged `1426dfd80` (the targeting override). Inputs are
`generated/native-cap-virtual4/0x1426aecd0.{c,asm}` and standard capture
`0066ad671efb445889acbd2b9cfc0c01` for the other two. Shared dependencies now
use the actual inventory interface, ability-end/FX, charge and sprint contracts.

Reload retains its captured pawn across notifications and +758, then freshly
reads ASC and activation handle. Duration scaling reads capacity before current
ammo; the complete ReloadType enum goes to OnLocalReloadStarted. Only its
nonzero check controls scaling. Charge support and automatic trigger checks
precede the actual native 900/mask80 write and complete charge-state action.

Auto-reload retains the equipped/interaction/emote/local sprint/predicate/targeting
gate order. The tag singleton offsets are 748 and 6f8. Instigator is reloaded
after ASC calls; EmoteStartTime is captured before virtual GetWorld; definition
is captured before virtual CanReload, while targeting is read afterwards.

The ranged setter captures targeting before the unconditional base call, sets
CE8 only for actual post-base change, and uses the original requested argument
for its timer gate. It dispatches the distinct ranged-row virtual +6b0. Assembly
1426dfe26 COMISS value,zero and JNC accepts ordered >=0; NaN and negative use
the exact .4000999927520752 fallback. The native D98 timer is explicitly zeroed
by constructor 1426bd2a0. The other two plain native fields start zero through
UObject allocation and remain shared with future original consumers.

Ranged reload 1426dd9e0 is a direct base thunk and receives no extra body credit.
Timer thunk 141b8e130 only tail-dispatches virtual +848: the weak UObject delegate
binds that virtual directly, without another synthetic or counted body.

FortWeaponAutoReloadHitchNative preserves the actual GHitchDetected false-to-true
scope transition and complete engine FLightweightStatScope::ReportHitch effect
(Core/Private/Stats/StatsMisc.cpp), including heartbeat, thread-name/string
lifetimes and log behavior. Diagnostic support receives no extra body credit.
Game and server targets enable the actual engine ALLOW_HITCH_DETECTION feature;
the scope fails closed if the engine detector is omitted. This changes engine
build configuration and requires a matching rebuild. Editor/debug configurations
which omit the detector are not accepted by this diagnostic support.

Independent source/assembly review found no admission blockers after correcting
the reentrant emote ordering. Generation, engine build and runtime behavior are
separate validation gates; source completeness does not establish movement parity.
