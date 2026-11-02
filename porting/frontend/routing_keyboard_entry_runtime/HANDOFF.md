# Keyboard action helper boundary

FortKeyboardBindingHelpers.{h,cpp} supplies source for the closed settings
and match phases, using actual UFortPlayerInput, UFortPlayerInputSettings,
UFortClientSettingsRecord, FInputActionKeyMapping and FInputActionBinding types.
Only this directory changed. No Native/generator/Engine edits, builds or commits.
The earlier viewport touch/axis fragments remain separate, unchanged source.

The new source supplies complete settings collector14200e000 and alias
lookup14200df90, reflected desired-profile getters141fe7d30/141fe7d00, cloud-state
predicate141e32a40, current-gamepad test142390950, the requested input-type prefix,
profile mapping selection and Engine mapping fallback phases of1423812b0, and
the concrete match/execute phase of14236b4d0. There are no injectable providers,
new input managers, default-success keys or replacement input processors.

Important contracts:

* Settings collection appends unique FInputActionKeyMapping values to the caller's
  existing array; it does not reset it. Equality includes action/key FNames and
  all four modifier bits. Its bool result tests the resulting array's length.
* Keyboard profile selection copies the first two mappings. Gamepad profile
  selection copies only the first, even when more mappings exist. Empty results
  preserve the supplied output keys. Diagnostic logging is omitted.
* Alias lookup picks the first matching ActionName. The alias's InputActionType
  field is not inspected. A successful alias overwrites only the primary key.
* Engine fallback copies every matching mapping in forward order, then selects
  its first two keys. Gamepad type1 accepts gamepad keys; keyboard type0 excludes
  gamepad and touch keys. Other requested values collect none. Duplicate Engine
  mappings are retained in that temporary array.
* Requested input type3 (ECommonInputType::Count) derives the actual CommonInput
  method when present; otherwise it stays3. The full resolver rejects Touch2
  before inspecting PlayerInput. Current-gamepad selection is a fresh independent
  check, so a forced requested type is not the sole profile-selection condition.
* Client record+884 joins reflected CloudFileState (metadata offset2180).
  Predicate141e32a40 accepts Saving1 or Idle3, rather than every nonzero state.
* The 0x48-byte records are FInputActionBinding, not a new struct. Match first
  checks KeyEvent, then resolver success and equality with either resolved key.
  bExecute controls ActionDelegate.Execute(Key). A match returns true even for
  an unbound delegate or query-only call. bConsumeInput is not read by this helper.

## Exact unclosed controller boundary

The complete controller resolver1423812b0 is not promoted by this component.
Its order is: requested-type resolution/touch rejection; cast real PlayerInput;
choose gamepad profile when current input OR requested input is gamepad, otherwise
try eligible local-player ClientSettingsRecord customization before the desired
keyboard profile; then current-profile key alias; then Engine fallback.
The gamepad branch supports an explicit named preset override. That lookup and
the eligible customization branch require their real producers and cannot be
replaced by unconditional desired-profile lookup.

Custom resolver141e28430 uses actual ClientSettingsRecord.UserBindingsPerSubGame
and current subgame (141ed4e30), performs a particular action-name remap via
globals14812cc98/14812cc90, searches FFortActionKeyMapping.KeyBind1/KeyBind2, then
falls back to keyboard mappings. The current-profile alias accessor141fe7d40
chooses by the controller's actual input method, independently of requested type.
Those joins, named-preset lookup141feab70 and their complete native source are
still required before using the new phases as a full1423812b0 replacement.

Predicates14238ead0 and1419dd310 are recovered exactly but not supplied as fake
live fields: controller.HUD+2a8 must Cast<AFortUIZone>; the first returns native
byte5d0, while the Athena predicate additionally compares FName5d4 with global
FNames14815bd80/14815bd88. The native state producers and the two FName identities
remain unresolved. No reliable mapped-image FName decoder was available, and
no guessed state names or always-false HUD flags were introduced.

14236ad20 additionally uses controller input binding arrays and special HUD/state
continuations. Therefore physical key callback145156590 and viewport key startup
must remain unpromoted until this genuine controller continuation is supplied.
The new MatchResolvedActionBinding source is explicitly the post-resolution
phase; it does not hide the missing resolver behind a callback or pass-through.

## Validation and ownership

helper-windows.json guards16 bounded original windows (1916bytes) with per-window
SHA256. No whole-image reads/hashes or original bytes are stored. Static source
joins were checked against target Engine InputComponent.h/PlayerInput.h and
generated reflection offsets/names. No additional per-helper test/build churn;
primary owns actual integration and build validation.

Preserve the user's bounded-read preference for code, assembly and output.
Keep the primary as planner/orchestrator/reviewer. Delegate bounded tasks with
fork_turns none: Luna medium small reads, Terra medium/high regular work, Sol
medium complex work, rotating at coherent component boundaries.
