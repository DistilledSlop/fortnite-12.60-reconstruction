# Sprint direction and cancellation

The two controller input predicates are complete original `14238e8e0` and
`14238f110`. Original captures live in local-model-package/captures:
`dd4549245f74465b85cf170c76b4126a`, `e084239f77984f0fa302df0a7dd53bf9`,
`1d17aa550e7f45d4b31b4099d9d8478b`, `a73fba9e52fa4240888cbb3ef62f5515`,
`26246b91c88943028812c4de2690dd9f` and `c486bacbc2804d8a9d44a78338fb6cb1`.
Independent review covers float unordered behavior, repeated movement getters,
pawn reloads around virtual calls, last-input rather than velocity, and original
unchecked component preconditions. The null-safe character-vehicle type test is
followed by the original unchecked pawn call.

The real native FFortAthenaVehicleInputState field begins at original 1194 and
ends before the next reflected state at 11c0. Its constructor explicitly zeroes
the 44-byte state; UObject zero allocation plus the trivial generated struct
preserves those initial logical values. Future input producers must share it.
The helper samples MoveForward, MoveForward_Gamepad and MoveRight in order,
then the actual UFortPlayerInput dead-zone field. The other branch uses the
native forward state, preserving its unordered-true comparison.

Three invoked vehicle bodies are complete: root primitive getter `142323920`,
velocity `1419193b0` and forward speed `141916010`. All 21 inventoried Athena
tables select the same +7a0 and +878 targets, independently reread in the flat
image during integration (`sprint-input-dispatch.json`). The secondary interface
at original +3b0/+4f8 selects 1419193b0. Velocity queries root primitive again,
GetBodyInstance(NAME_None,true), its actual bSimulatePhysics, then virtual
GetComponentVelocity; otherwise it returns existing native c40 velocity storage.
Forward speed preserves the earlier component pointer, dots its forward vector
with that velocity, and multiplies by .036. No additional validity guard is added.

The four cancellation bodies are `1423b8500`, `1423b65b0`, `1423909c0` and
`1423508d0`. Inputs are native-cap-helpers30, native197-fresh-callees,
native-cap-helpers32, and captures 2f912fc32b104d449dd86833d589fdf7 and
b67da34add9142fea37826078842fea9. Pawn 522/mask10 is the actual bPrimaryInputHeld
per pinned bool_bits.json. The touch predicate preserves both virtual GetWorld
calls, the tearing-down world check, LocalPlayer cast and actual
UCommonInputSubsystem lookup/current Touch=2. Class accessor 14081de90 matches
pinned subsystem reflection. UWorld shipping layout has DemoDriver f0,
ViewLocations 108..117, Feature/Group/WorldType 118..11a and flag bytes 11b..11d;
11d/mask20 is bIsTearingDown. Earlier replay interpretations were incorrect.
FortniteGame already depends on CommonInput; body include inference now admits
that existing acyclic edge as it already does for reflected types.

Registrar 14031d540 registers mutable int32 Fort.SwimSprintByDefault with the
original help text and flags zero. Backing RVA 8157d00 is beyond original PE
.data raw end 80ce000 but inside its virtual tail, proving initial zero.
FortSwimSprintCVar is synthetic infrastructure without original-body credit.
Complete source does not establish producer lifecycles or walking acceptance.
