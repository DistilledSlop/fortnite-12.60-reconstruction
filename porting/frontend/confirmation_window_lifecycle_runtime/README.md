# Confirmation window lifecycle candidate

`declarations.inc`, `members.inc`, `button-info-members.inc`, and
`definitions.inc` are class-specific source fragments. They use the reflected
window fields and the restored dialog constructor and delegates. They are not a
complete connected component: the BotClickButton controller flag producers and the load guard Slate construction
and loading update remain unresolved. The shared latent handle allocator is
implemented in `latent-handle.cpp`. No
finite promotion policy is provided until those boundaries are closed and
integrated validation succeeds. `lifecycle.h` is the earlier generic control
flow sketch, superseded by these fragments.

Root entry 144f664f0 queries `UWidget::IsVisible` (Slate cached-widget
visibility), copies the +0x4b0 dialog, resets it, closes the window, resolves
FortGlobalUIContext for the owning local player, and conditionally dispatches
result 3, NAME_None, false, invalid handle. 144ce33d0 resets the handle and
executes the ordinary callback followed by the advanced callback. Keeping the
copy alive across reset, close, and both callbacks preserves reentry. Decline
144f5fce0 uses the same sequence with result 1 and ignores its button argument.

Reset 144f75f90 assigns a fresh dialog, removes native button bindings, empties
both entry boxes with delete=true, repopulates the optional panels, refreshes
both texts and description scroll visibility, then updates the icon and its
visibility. It removes the bindings again when close 144f55ae0 runs; close
deactivates only when no native close delegate is bound, otherwise broadcasts
the native one-parameter multicast (original +0x5c8). The TMulticastDelegate
performs its own reverse iteration, lock and compaction. Button pairs are 16
bytes in the original: reflected Button and unreflected FName at +8.

BotClickButton is reflected exec 145218540, registration creator 145215d60;
the function flags are 0x04080401 (final, native, private, BlueprintCallable), there are no parameters and the exec tail-calls 144f54030. Its first lower
boundary tests whether GetOwningPlayer() is AFortPlayerControllerAthena and
whether byte +0x1838 or +0x1818 is nonzero. Reflected AFortPlayerController::IsClientBot exec 142a47ff0 computes the identical expression. Its two native flag producers remain unknown; `controller-members.inc` is an unintegrated candidate, not permission to silently initialize the flags without their producers. 144f5fbc0 checks button and visibility, finds its associated name and takes
the ordinary confirmation path or latent path according to description +0xe6.
The latent path disables the button, resolves context, sets and allocates the
window handle, calls the live description's ordinary then advanced callbacks,
resets the simple entry box and enables the load guard's loading state. The
advanced handle is read after the first callback, preserving reentry. The
allocator at 141f87900 uses a shared signed global initially -1, increments it
and maps overflow to 0. It must be shared with all other latent-action producers.
The load-guard boundary 1407f7e20 updates a Slate object when available and
must be provided by the load-guard component. The fallback calls decline.

The local-player subsystem lookup is supported by original getter 14523f270
and its FortGlobalUIContext/FortLocalPlayerSubsystem hierarchy. The original
144ade110 also resolves a widget through its outer chain if needed; the present
method always receives UFortConfirmationWindow, itself a UUserWidget. Engine
visibility and native multicast behavior should be verified in the integrated
build. No generator, Native, Engine, build or staging files were changed here;
no connected build or UE runtime test was performed.

Activation override 144f6bb10 is translated in `definitions.inc`: it removes
old buttons, creates actual entry-box UCommonButtons, sends Blueprint configure
events, binds native click callbacks with button payloads, records each
ResultName, creates the decline button, refreshes display, calls the superclass
activation, clears the spinner, and performs focus and scroll-widget handling. The focus wrapper identifies the optional campaign NavigationManager
and otherwise calls UWidget::SetFocus. Its manager-specific 144db14e0 operation
and scroll-widget helper 144e1c6e0 still need their real lower implementations.
UCommonLoadGuard requires a real SLoadGuard built with its configured text,
style, alignment and brush and its ULoadGuardSlot content; the existing lazy
image-only construction does not implement these fields. Do not replace this
with a blank widget or no-op spinner.
