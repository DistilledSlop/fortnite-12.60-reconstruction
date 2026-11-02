# Root input configuration and focus application

Unregistered, unbuilt source/evidence candidate. Only this directory changed;
no Native, generator, Engine, build or commit changes. The small source candidate
closes one concrete Slate-to-UObject ancestor helper. The root's application
chain is substantially recovered below; its remaining native override/enum and
platform-policy boundary must be implemented before promoting the root.

## Real widget types and the nested optional correction

Reflection class block145a004e0, class getter140805ec0, and Bool setters join:

| Widget offset | Real property | Setter |
| --- | --- | --- |
| 25a | bSupportsActivationFocus | 140809010 |
| 25b | bIsModal | 140808f90 |
| 25c | bAutoRestoreFocus | 140808ee0 |

The generated reflection JSON supplies these setter identities; the guarded
original setters each write byte1 at the listed offset. This closes the unnamed
25a selection gate in the preceding activation handoff.

Widget vtable1459ed638+470 points to1407c25c0, which returns a **nested optional**:
payload byte+0, inner is-set+1, outer is-set+2. Its proven default clears only
outer+2.1407bd840 uses the inner optional when outer is set, even when the inner
is unset; it inherits from the weak parent only when outer is unset. An explicit
empty inner therefore clears an inherited value. The node result and router+d8
multicast argument are the two-byte INNER optional. This is not a flat optional
widget override. The one-byte payload's exact enum identity and original method
name remain unresolved; do not declare it as an arbitrary enum or a bool.

The router+d8 delegate iterates backward, constructs an inner optional argument
for each live invocation, tracks failed/dead invocations, decrements nesting,
then invokes the existing deferred compactor. Its semantic argument is an
optional one-byte value; the machine's indirect argument alone does not prove
whether the original declaration takes it by value or const reference.

Widget+478 points to1407c26c0. This returns an optional three-byte input config
by clearing is-set+3. This is a real empty default, not license to replace all
derived implementations with empty providers.1407bd980 uses a present widget
config, otherwise recursively pins/inherits the parent config, otherwise
returns bytes{0,0,1}. Its result is a concrete config, not an optional.

Config byte+1 is EMouseCaptureMode, byte+2 is the requested hide-cursor-during-
capture bool. Byte+0 is the input-mode notification value (exact enum names and
values not yet joined). Do not infer menu/game behavior from byte+0: the observed
viewport branch below tests byte+1. No original struct identifier is claimed.

## Router application1407f4770..1407f4cbd

Router+80..83 is TOptional<the three-byte config>. If already set and all three
incoming bytes match, return immediately. Otherwise record old is-set and old
byte+0 (use2 when unset), then assign the new optional BEFORE external effects.
The incoming config remains the source of later reads, rather than a snapshot
of the newly assigned member.

Router UObject+20 is the owning ULocalPlayer. Resolve its viewport+70; require
GetGameViewportWidget144214ff0 nonempty, then obtain its player controller through
router virtual+150 followed by14447f220. Missing viewport/widget/controller skips
viewport effects but still reaches the byte+0 notification comparison.

Read the viewport's previous capture mode through viewport's secondary virtual
+180; record whether old capture was1 or2. Then SetCaptureMouseOnClick14422dbd0
to config+1 and SetHideCursorDuringCapture14422d7e0 to config+2 AND NOT
router policy1407fb160. Setter strings prove MouseCaptureMode and MouseLockMode;
1444889f0 explicitly logs Player bShowMouseCursor Changed.

For capture0,3,4: SetMouseLockMode(0), set player bShowMouseCursor=true, then
ULocalPlayer::GetSlateOperations() (local-player+1a0) ReleaseMouseLock and
ReleaseMouseCapture. These mutate the existing cached FReply: weak mouse-lock
and captor are reset; release bits are set; high-precision bit is cleared.
For unsupported values>4 skip the rest of this viewport branch.

For capture1,2: SetMouseLockMode(1), set player bShowMouseCursor to policy1407fb160,
then on that same FReply UseHighPrecisionMouseMovement(viewport),
SetUserFocus(viewport,EFocusCause::SetDirectly,false) via142f6df60,
and LockMouseToWidget(viewport) via1407adfb0, in that order. The original also
retains/releases copies and intermediate weak assignments; calls consume their
by-value shared arguments. A newly constructed FInputModeGameOnly/UIOnly cannot
replace these exact mutations of an existing reply.

In the capture0/3/4 branch only, when a previous config existed, previous capture
was1/2, and1407c3b10(router)->byte79 via14081b540 is not2: obtain the local-player
viewport geometry1442f5e70, pin viewport's scene-viewport weak+f0/+f8, query its
size via virtual+10, compute the transformed center and call1430314b0 to move the
cursor. The setting getter/type at1407c3b10 and scene-viewport call still require
a source join. Do not replace with a guessed screen-size center.

Finally, only if old byte+0 differs from incoming byte+0, broadcast incoming byte
to router+c0. Iterate backward with nesting/deferred compaction as for+d8. This
still happens when viewport/controller resolution failed. First set uses old
value2, so incoming2 on the first set produces no notification.

## Saved focus1407ad210 and ancestor helper1407be930

Node+80/+88 is TWeakPtr<SWidget>, not a UObject weak pointer or arbitrary token.
Resolve owning router then local player; derive player index by searching its
game-instance local-player array (1442f3c00 /1407d6e10), use-1 if absent. Obtain
current focused widget from global Slate virtual+160 with that index.

For nonempty current focus,1407be930 consumes a shared copy and walks its Slate
PARENTS, starting with GetParentWidget, never testing the supplied widget itself.
When GetType()==FName("SObjectWidget"), inspect SObjectWidget::GetWidgetObject(),
Cast<UCommonActivatableWidget>, then require GetOwningLocalPlayer()==owner.
The first activatable ancestor belonging to another player terminates with null;
it does not continue upward. CommonActionFocusAncestor.h supplies this helper
with actual SObjectWidget/UCommonActivatableWidget/ULocalPlayer types. Its name
is descriptive. Engine joins: SlateCore/Public/Widgets/SWidget.h GetParentWidget
and GetType; UMG/Public/Slate/SObjectWidget.h GetWidgetObject.

1407d00e0(widget,node) returns true when widget equals resolved node weak+58, else
recurses over every child raw identity in forward order. It has no activity,
supports-focus, or weak-validity gate and no retained child snapshot.

Save replaces weak+80 only when current focused raw identity differs from the
previously PINNED remembered raw identity AND (ancestor is null OR ancestor is
one of this node/subtree's widgets via1407d00e0). Thus no current focus can clear
a previously remembered live focus; an ancestor outside this subtree preserves
the previous remembered widget. Expired previous weak compares as null.

## Root focus1407bef20..1407bf51c

Pin root selected-node+c8/+d0, resolve its widget+58, root owning router+28 and
local player+20. Original assumes the selected-node contract here: do not add an
empty-selection success path. Derive local-player index as above and keep the
local player for pending Slate operations.

1. Only when widget bSupportsActivationFocus AND bAutoRestoreFocus, pin selected
   node's remembered Slate widget+80. If nonempty, immediately call
   FSlateApplication::SetUserFocus(index,remembered,EFocusCause::SetDirectly),
   remembering its bool result. Failure does not proceed to another target.
2. Otherwise dispatch widget virtual+480. Its base target1407d1ee0 tail-calls
   event wrapper140808b40, whose reflected event is BP_GetDesiredFocusTarget.
   A nonnull UWidget result receives UWidget::SetFocus143d8a480. This path leaves
   the direct-Slate-success flag false, regardless of internal focus success.
3. With no desired target: obtain current Slate focus and pin the local player's
   pending FReply focus-recipient weak+1e8/+1f0. If either is within the selected node's
   exclusive Slate subtree via1407cf170, keep focus and set no direct-success
   flag. This is SlateOperations+48/+50 (FReply::GetUserFocusRecepient), so pending focus is considered before current focus.
4. Otherwise if selected widget IsFocusable (byte1b4 bit2), UWidget::SetFocus
   that widget. Otherwise, when a viewport exists, obtain its cached viewport
   widget and attempt direct Slate SetUserFocus(index,viewport,SetDirectly).
   Missing viewport just falls through without success.
5. Release focus temporaries; only when the direct-Slate-success flag is true,
   call local-player SlateOperations.CancelFocusRequest142f4b5b0. This clears
   set-focus AND clear-focus request bits and resets the pending weak focus
   recipient. It does not release mouse capture or clear actual Slate focus.

1407cfcd0 is the strict descendant test: require supplied focus nonempty and
valid node weak widget; get its cached Slate widget143d5a140; walk supplied
focus's PARENTS and compare against that cached widget. Self is excluded.
1407cf170 requires that test succeed for this node, and that it fail for every
immediate child with a valid widget and bSupportsActivationFocus. It does not
check child activity or call the recursive1407cf170 on children. Therefore
keeping current focus must use this exclusive subtree test, not ordinary
inclusive ancestry and not the UObject subtree membership helper1407d00e0.

## Precise next boundary

The overall order remains1407aa8d0: nested-optional override derivation and+d8
broadcast, fresh selected virtual+18 gate, derive config, fresh owner resolution,
apply config1407f4770, then apply focus1407bef20. Delegate reentry can affect the
fresh gate. Do not merge the three delegates(+d8 optional,+c0 mode,+b8 root
selection) or replace actual application with remembered selection.

Before promoting real root methods: join widget virtual+470 payload enum and
actual override producers, config byte0 enum/struct and derived+478 producers,
router virtual+150 controller-id provider,1407fb160 platform cursor policy, and
1407c3b10->byte79 input-method source.1407fb160 combines Slate helper143014960
(two bytes+420/+421), viewport144217cd0, and global byte1480d1c81; these are real
producers still requiring names/ownership, not an injectable boolean hook.
Concrete route-class lifecycle/delegates/tree creation remain with earlier
handoffs. Recovering their source is separate from inventing an abstract router.

## Validation and commands

`python3 porting/frontend/common_action_route_focus_runtime/replay.py` PASS:
24 bounded original-image windows (6215 bytes),3 widget virtual slots. Static
evidence only; source candidate has not been built or run. No raw original-image
bytes are stored. Replay uses os.pread/SHA256 per bounded window. Decoder used:
`python3 /mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py VA_HEX:SIZE_HEX`.

Primary retains integration/review and Native/generation/build/commit ownership.
Bounded delegation: Luna medium small reads, Terra medium/high regular work,
Sol medium complex work; fork_turns none. Keep reads bounded and rotate here.
