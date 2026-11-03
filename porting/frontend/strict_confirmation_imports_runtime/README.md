# Confirmation async import candidate — not yet connected

The custom-input factory, activation, result callback and resolver code are
class-specific translations with real descriptions, action callback delegates,
external-input state, and transient action lifetime. Reflected exec 1451f76d0
is static, takes the parameters declared in `action-members.inc`, and targets
144b1aeb0. Factory 144b1a460 does not register the action with a game instance,
activate it, or root it. Activate 144ae8b90 binds a weak UObject callback and
calls the native manager interface; a missing pair reports Unknown. Callback
144afc830 restores only an external input suspension that this action cleared.

The files are not ready for promotion: the manager pair's virtual source in
UFortUIDirectorSubsystemBase and concrete manager-owned storage remain to be
joined, the IFortUIManagerInterface must be installed on all original owners,
and the manager/root queue presentation and completion bindings must be
implemented. `NativeIsExternalInputSuspended` exposes the existing actual +FE
state, rather than conflating it with internal suspension. No build was run.
The checked-in CommonUI queue pool supplies downstream widget ownership; it is
not itself the missing Fortnite dialog queue and must not be duplicated.

Additional root fragments now implement the startup/Athena append, presentation,
and completion sequence. Original 1451611b0 binds 14514fda0 to the window's
native deactivated multicast during NativeConstruct; that handler hides the
window, removes the first queued entry, and presents the next. The empty path
broadcasts a native bool multicast based on active modal and panel visibility.
The active-widget lookup 1407c1910 uses the real animated switcher active Slate widget, requires the SObjectWidget type, and casts its UObject to UCommonActivatableWidget; `queue-active-widget.cpp` supplies that boundary. The shared global idle flag begins at 1 in mapped image,
and any other producer/consumer must use the same getter/setter storage.
