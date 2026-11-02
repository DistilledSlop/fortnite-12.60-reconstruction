# Original node/root activation and Slate preprocessors

Unregistered, unbuilt source/evidence candidate. Only this directory changed.
No Native, generator, Engine, build or commit changes. This boundary closes the
node gate algorithm and identifies its +98 payload and concrete Slate calls.
Root activation reaches real focus/input-config application: that dependency is
the next implementation boundary, not permission to install empty callbacks.

## Node virtual+30: 1407f5d50..1407f5ebd

Compare incoming bool against byte+90 first; equality returns without inspecting
widget, processors or children. On enabling only, require valid weak widget+58
through 142ee7880, resolve through 142ee5dd0, and require widget bIsActive+288.
Failure returns without any write or child dispatch. Disabling has no widget
validity/activity gate. Then write byte+90 BEFORE all external effects.

Capture +98 array start and end (num+a0, stride0x18). On enable call
FSlateApplication::RegisterInputPreProcessor(entry.InputProcessor, entry.Index)
for every entry, ignoring bool return. On disable call
FSlateApplication::UnregisterInputPreProcessor(entry.InputProcessor) for every
entry. Each call copies/retains the shared processor; the by-value callee consumes
that temporary. Fetch the application singleton anew for each call, not once.

AFTER processors, capture children+70 start/end (num+78, stride0x10). For each
entry, copy its shared pointer, dispatch its concrete virtual+30 with the original
requested bool, then release the copy (including possible object/controller
destruction) before advancing. The parent owns no retained child-array snapshot.
A child independently rechecks its weak widget and activity on enable. Do not
propagate the parent's current gate read back after a callback, prefilter children,
use a cached virtual+18, or erase a failed processor registration from storage.
Original captures each array range once; mutation/reentry is not made safe by
an invented iteration queue in the node.

## +98 payload and production

This is an input-preprocessor entry, not a delegate/callback object. Recovered
source type is `{ int32 Index; TSharedPtr<IInputProcessor> InputProcessor; }`,
with index+00, shared raw+08/control+10 and stride0x18. The small header here
provides this concrete payload and ABI guards; its identifier is descriptive.
The 4 bytes after Index are alignment padding, not another recovered field.

Append 1407aa060(this, const shared processor&, int32 Index): append an entry
unconditionally, retaining the processor; freshly dispatch virtual+18; if true,
register the supplied processor/index immediately. There is no node-level
duplicate check, null gate, or cleanup based on Slate's result. Thus null entries
are stored; null registration later returns false inside Slate.

Router producer 1407ecc70(router, widget, processor-ref, index) acquires a route
through 1407beb40 and calls append at 1407eccb1. Without a route, its existing
owner fallback stores a different `{shared processor at+00, int32 Index at+10}`
0x18-byte entry in owner-record+20. Transfer 1407e53d6..1407e5447 copies each
owner entry, reads its index+10, then invokes node append at 1407e5414; the layouts
must not be memcpy-interchanged. Caller-level concrete producer object remains
outside this bounded recovery. Both record paths accept real caller-supplied
processors; there is no reason to synthesize another input handler.

14302d500 is FSlateApplication::RegisterInputPreProcessor; 143036460 is
UnregisterInputPreProcessor. Structural source join:
`/opt/fortnite/UnrealEngine/Engine/Source/Runtime/Slate/Private/Framework/Application/SlateApplication.cpp`
lines3419..3434 and6634..6684. Original singleton148271318 and helper+570 match
those exact routines, including the unusual indexed insertion behavior below.
The machine implements the default non-thread-safe shared-pointer counter path.

Register rejects null raw processor. In helper+570, search the live list by raw
pointer; if absent, AddInternal immediately or add processor->Index to the pending
addition map when iterating (byte+580). Always remove all matching entries from
pending-removal array+588, even if already present. Return !alreadyPresent.
Unregister during iteration appends a retained processor to pending-removal;
otherwise stable all-match removal from the live list. Always remove the key
from pending-addition map+598. Unregister does not first reject null.

AddInternal143002650: Index==-1 appends. Otherwise if Index>=Num, SetNum(Index+1)
with zero/empty shared slots, THEN Insert at Index. This yields length Index+2
in that case, exactly matching existing Engine source. Do not simplify to indexed
assignment or clamp all negative values. Helpers140bbc340/140bbc4a0 are live/pending
shared-pointer removal by raw-pointer equality; this is shared ownership, not
a weak callback subscription.

## Gate producers connected to actual state

1407cca90 is the node's activation continuation: execute bound callable+38 if
its virtual+28 reports bound, then pin weak parent+60/+68 and freshly call its
virtual+18. Only a valid parent whose predicate succeeds causes this node's
virtual+30(true) at1407ccb1d. Afterward1407b94e0 controls whether it locates the
root with1407c46e0 and submits this node to1408030f0. Callable+38/+48 types and
1407b94e0 are still the widget-native delegate boundary, not reconstructed here.

1407ccc50 performs counterpart callable+48, then if byte+90 is true calls this
virtual+30(false) at1407ccc9d. Its remaining parent/root reselection is not
replaced. Node destructor14079988c calls this continuation conditionally.

Router1407f4510 owns selected shared root at+60/+68. Before replacing it, remove
old root's single callable+b8 and call old root virtual+30(false) at1407f4587.
Only router byte+98==0 and byte+99!=0 allow installing the incoming shared root;
otherwise clear selection. On installation call new root virtual+30(true) at
1407f4614 and bind root+b8 notification back to router1407c9c40 (address in thunk
1407f464d). Those router gate producers/binding lifecycle are the next boundary;
setting every root's +90 true would bypass actual selection.

## Root virtual+30: 1407f5ec0..1407f6044

First compare requested bool to byte+90; equality returns, including no refresh.
Otherwise call base node1407f5d50 with the unchanged bool. Root branches on the
REQUESTED bool, not a post-base read of the gate.

Disable: only if weak selected-node+c8/+d0 has nonnull raw/control and a positive
strong count, pin it, invoke1407ad210, release pin, then call1408030f0(root, empty
shared node). If the weak selected node is absent/expired, return without clearing
the weak fields or calling1408030f0. This ordering must not become unconditional
Reset.1408030f0 can itself call1407ad210 for a different previous selection; do
not coalesce the two calls on assumptions about idempotence/reentry.

Enable: independently validate root widget+58 and bIsActive+288 AFTER base
effects/child propagation. On success pin shared-from-this weak+08/+10, pass a
second shared copy by value to1408030f0, release the first, and if that helper
returned false call1407aa8d0(root). A failed/expired weak-self pin is passed as
empty, not converted to a raw root. Pin releases can run destruction.

1408030f0(root, shared candidate by value): reject nonempty candidates whose fresh
virtual+18 returns false. Otherwise select via1407be240(candidate), or use empty
for empty input. Compare selected raw identity against freshly pinned root+c8
weak identity. If equal return false. If different, pin/invoke1407ad210 on the
old selection when valid, assign selected weak identity+c8/+d0, call1407aa8d0,
then execute root+b8 via1407bc690, release temporaries and return true. An expired
old raw weak value compares as null. No route notification+a8 is substituted.

Selection1407be240 requires widget weak validity, bIsActive+288 and widget byte+25a.
It recurses over shared children in forward order. Among nonempty recursive
results it retains the result with strictly greatest1407c3c30 value; ties retain
the earlier child. If no child wins, return the candidate itself.1407c3c30 returns
cached Slate widget+148 when available via143d5a140, else -1. Byte+25a is not
given a source property name here without a separate reflection-offset join.

## Exact next boundary, required before root source promotion

1407ad210 saves focus-like state into node weak+80/+88. It resolves router weak+28,
uses router UObject+20,1442f3c00/1407d6e10 local-player lookup, global Slate
virtual+160 with player index (or -1), then1407be930 and1407d00e0 to decide whether
to replace weak+80. Its complete original window is guarded. These are real Slate
user/focus dependencies, not a void teardown hook.

1407aa8d0 pins selected+c8, resolves owning router, computes optional byte through
1407bd840 (widget virtual+470, else parent inheritance), broadcasts optional byte
to router+d8 delegate with original reverse/deferred-compaction semantics, THEN
rechecks selected virtual+18. On success derive a three-byte config1407bd980,
resolve owner again, call1407f4770(owner, config), then1407bef20(root). On inactive
selection it only follows logging/release after the delegate.1407f4770 compares
and writes router optional config+80..83, then performs controller/input effects;
1407bef20 and the complete config/focus chain remain to recover. No implementation
of root activation can replace these with remembered selected-node state.

The root method/control flow is recovered, but native delegate types, router
config application and focus producers are not all closed. Therefore no fake
route class, abstract callback hook, or executable root setter is supplied.
Continue bounded at1407bd980/1407f4770/1407bef20 and1407be930/1407d00e0; integrate
node setter only as a member of the real route class with its actual children.

## Validation

`python3 porting/frontend/common_action_route_activation_runtime/replay.py`
replays only bounded original-image windows using os.pread and SHA256 per window,
plus node/root+30 vtable slots. It validates static evidence; no Unreal build or
runtime parity is claimed. No original image bytes stored. Decoder is
`python3 /mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py VA_HEX:SIZE_HEX`.

Primary retains planning/integration/review. Bounded delegation: Luna medium
small reads, Terra medium/high regular, Sol medium complex; fork_turns none.
Keep reads/output bounded, rotate at the stated next boundary.
