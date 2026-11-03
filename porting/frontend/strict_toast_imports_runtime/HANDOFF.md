# Toast imports: incomplete recovery, generation blocked

This component does **not** close either import. `policy.apply` deliberately refuses
both owners rather than emitting incomplete functions. It is not integrated into the
generator. The original image windows and filtered native-function records are saved
alongside it; `validate_source` uses only bounded reads.

The reflection entries resolve as follows:

* `UFortFriendSubscriptionToastWidget::SetToast(UFortUINotification* ToastNotification)`:
  flags `0x4020401`, exec `0x14523d570`, body `0x144d97670`. The argument class
  thunk `0x142b26c60` ultimately resolves a constructor naming `FortUINotification`.
* `UFortPlayerPowerRatingToast::HideToast()`: flags `0x4080401`, exec `0x1452a50b0`,
  body `0x144e0c9c0`.

Friend SetToast stores the notification at `this+0x280`, sets hold-key binding
name `SubscriptionNudgeHold`, branches on notification `bHasAction` at +0x58,
sets text from notification +0x40, configures button/switcher state, installs a
world timer using `DisplayTime` at +0x294, and calls the open animation event
wrapper `0x145238060`. The action branch has additional player/controller input
and optional social behavior; this is not yet identified well enough to implement.
Its animation-finished callback `0x144d7e4b0` only broadcasts `OnToastFinished`
when `this+0x288` is set, clearing that byte first.

Power HideToast tests a weak object at `this+0x290`. If valid, it removes the
widget's delegate bindings from that object +0x78, then from the registered player's
+0x250 and its optional profiles +0xe8; it resets the weak reference and always
broadcasts `OnToastFinished` at +0x258. The multicast producers and bindings must be recovered before faithfully implementing teardown. Recover
power SetToast (`0x144e1cd20`), constructors, lifetime and delegate producers to
establish those identities; do not substitute generic delegates or skip unbinding.

No build or gate was run. The exact callback types, timer callbacks, lifecycle,
constructor defaults, and action behavior remain outstanding.

## Further power-toast recovery

The weak object is `UFortTeamMemberCampaign`, identified via its actual class
constructor and vtable. `0x142292260` resolves a `UFortRegisteredPlayerInfo` by
comparing the team's virtual identity with `UFortGameInstance::RegisteredPlayers`;
`ToastRegisteredPlayerLookup.cpp` supplies the corresponding real lookup once the
team identity and game instance are supplied. The owner at +0xd8 is
`CommonPublicProfile`, and the owner at +0xe8 is `AccountProfile`. Both profile
subscriptions use the `UMcpProfile` native multicast at +0xe8. The registered
player multicast is at +0x250. The callback for that multicast (`0x144e00850`)
removes its own bindings and retries setup; the profile callback (`0x144e07780`)
does the same for the profile passed as a bound payload. Team multicast +0x78
binds `HideToast` itself.

Campaign team identity virtual +0x268 is `0x142292160`. It prefers a weak object
at +0x90, then `SocialUser`, then a weak object at +0x38; the three routes and
native fields must be joined before lookup can be called faithfully. Team member
constructor `0x142afa270` initializes weak references +0x30/+0x38 and native
multicasts +0x48/+0x60/+0x78. Campaign constructor `0x142af85c0` initializes the
weak object at +0x90 and additional state at +0x98/+0xa0/+0xa8. The missing
multicast types, producers, callback argument types, and setup continuation at
`0x144e22f32` are still required; no substitute completion is emitted.

The campaign identity sources are now identified by reflected `UniqueId` offsets:
`ALobbyBeaconPlayerState` (+0x230, shared pointer at +0x238) for team weak +0x90,
and `APlayerState` (+0x248, shared pointer at +0x250) for weak +0x38. The
`ResolveToastTeamMemberId` helper implements these branches and the social-user
primary ID branch in the original order. This source requires Party and Lobby
module dependencies. It does not supply the missing team-member fields or callers.

The friend constructor body `0x144d6f540` initializes the completion multicast,
timer and notification storage to zero, close-state byte false, weak input
component empty, and the config defaults DisplayTime 5.0, HoldDuration 1.0,
DebounceHours 336. The corresponding constructor fragments are included for
later integration; UE initializes zeroed UObject storage and the normal member
constructors initialize the weak pointer, timer handle and multicast.

Friend display timer callback `0x144d7fd10` checks the retained notification and
its action flag, performs an analytics operation when notification +0xe0 and
weak context +0x100 are present, then calls close helper `0x144d74030`. The
close helper asks the owning player controller to release its input component
when its concrete controller and component are present, sets close state true,
clears the retained notification, and invokes `OnPlayCloseAnimation`. Animation
completion then conditionally broadcasts as described above. Omitting the input
release or analytics would leave a materially incomplete lifetime.

`friend-animation-*.inc` provides the recovered animation-finished callback and
its close-state field. The close path must set that field before the close event;
these fragments alone do not supply the close path or timer callback.

Controller type at the close helper's cast `0x142a53450` is
`AFortPlayerControllerAthena`; the reflected component at +0x3138 is
`SubscriptionNudgeInputComponent`. The animation fragments now include the
actual close helper and retained notification, needing `AFortPlayerControllerAthena.h`
and `UFortUINotification.h` in the generated class implementation/header. The
close helper is not wired to the yet-unrecovered display callback.

## Continued closure recovery

The input path is now identified beyond its first missing call. SetToast binds
`SubscriptionNudgeHold` pressed to `0x144d80480` and released to `0x144d806b0`,
using the real controller input component only when `HasBindings()` is false,
then pushes that component on the controller. This test is UInputComponent's
seven native binding arrays, not a toast-specific flag. Controller input-mode
query `0x14238ead0` checks its player input class and its real byte at +0x5d0;
its identity must be recovered before choosing Button_Cursor versus Button_Hold.

Pressed installs a non-looping timer at toast +0x278 for HoldDuration. Its
callback is `0x144d7c410`, and it starts the keybind progress operation
`0x144de3820` with `SubscriptionNudgeHold` and HoldDuration. Released clears
that timer if active, resets its handle, calls the distinct keybind progress
operation `0x144de3aa0` with false, records the notification response
`Dismissed`, then closes. The hold timer callback clears the **display** timer
at +0x270 (not its own +0x278), resets that handle, invokes the notification's
native virtual at +0x270 if retained, then closes. These operations cannot be
replaced with widget enable/disable or CommonUI action delegates.

The display timeout records `AutoClose`. Both analytics routes require native
notification +0xe0, weak context +0x100, and optional native object +0xf0's
string at +0x18. They call `0x1417ef620` on context +0x258. That body constructs
three analytics attributes from the recipient ID, response string, and optional
payload. The notification +0xe0 object is therefore also an online unique ID;
it is not a UObject notification subtype or an arbitrary analytics callback.

SetToast's optional social route obtains the `MCP` online subsystem and shared
interface (`0x140d0e9e0`, `0x140d126c0`), then calls `0x140cd1e80` with native
unique IDs at notification +0xd0 and +0xe0 and a real shared delegate. The
callee validates both IDs through virtual +0x28 and updates MCP's native map
at +0x598. Its exact interface method and completion contract remain missing.
An empty completion owner, generic USocialUser event, or fake notification
getter would not implement this path.

Power setup's binding phase at `0x144e22b70` has now been followed through
`0x144e22f32`. It binds team +0x78 to HideToast unconditionally while the weak
team member is valid. It looks up the registered player, checks the actual
ability-system actor through `0x141f5d880` (weak +0x20c), and, if absent, binds
registered-player +0x250 only when not already bound to this toast. The bound
payload is the registered player. Each existing profile is checked with
`0x141104100`; an unloaded profile is bound once through profile +0xe8 with
the profile as a bound payload. The native multicast has **two runtime
arguments**, confirmed by invocation `0x144d32260` and safe invocation
`0x144d33a90`; `0x144e07780` reads the profile payload from r9. Do not infer
a zero-argument delegate from that callback ignoring its runtime arguments.

When all prerequisites are ready, setup removes the team's binding, obtains
the real power-stat singleton (`0x14250cde0`) and widget context
(`0x144ade750`), calculates member, party and delta power using
`0x1425159f0`, `0x142516630`, `0x1425108f0`, and `0x142508f60`, then formats
the actual toast text and invokes ShowToast. If the weak team or registered
player is unavailable, it calls HideToast. This is a real setup continuation,
not an event that can be represented by declaring empty owner multicasts.

New bounded source windows cover the recovered input callbacks, setup binding
phase, profile callback dispatch, loaded query, actor query, MCP operation
entry and analytics entry. `validate_source` checks them alongside prior proofs.
No complete export is emitted: missing producer contracts remain a hard gate,
and the recovered input/keybind/notification/MCP/analytics chain requires source
beyond the reflected toast owners. No build, generated-source edit, or strict
gate success is claimed.
