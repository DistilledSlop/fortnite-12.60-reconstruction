# Original concrete route identity

Evidence only, unregistered and unbuilt. Only this directory changed. No Native,
generator, Engine, build or commit changes. This closes the virtual +18 predicate
and weak +28 owner identity requested by the route-association handoff. It does
not supply a replacement route class or a router implementation.

## Actual runtime hierarchy and notification

Three concrete runtime vtables are established by constructor writes and slots:

| Runtime identity (descriptive, not recovered C++ name) | Vtable | Virtual +18 |
| --- | --- | --- |
| Base binding collection | `1459ecdd0` | `1406801d0`: returns true |
| Activatable widget node | `1459ecdf8` | `1407cff20` |
| Root activatable widget node | `1459ece30` | `1407cff20` |

The node predicate is **exactly** `node byte+90 != 0`, then valid weak widget+58
via `142ee7880`, then resolve the same weak widget via `142ee5dd0`, then require
widget byte+288 != 0. It returns false as soon as any gate fails. Do not replace
it with true, false, just widget activity, or a remembered result. Root inherits
the same implementation. Base returns true: the router's standalone base
collection is a real case, not an inactive-node fallback.

Widget byte+288 is `UCommonActivatableWidget.bIsActive`, exposed by IsActivated:
property setter `140777600` writes it, exec `14080bae0` reads it. The prior
`../common_activatable_panel_native_events.md` metadata join already binds that
setter to the original Bool property. This folder pins both machine witnesses.

Association `1407a91aa` freshly dispatches this virtual, and only on success
resolves route weak+28 then broadcasts owner delegate+a8. The weak owner is
**UCommonUIActionRouter**, not the binding widget, local player or subsystem
manager. The original contains no extra null fallback between resolve and +a8.

## Owner and construction proof

Class getter `1408132b0` uses UTF-16 `145a08ae2 = CommonUIActionRouter` and ctor
thunk `1408135e0`, which loads object-initializer's object then jumps to
`140797b40`. That router constructor installs vtable `1459eee30`, allocates an
in-place shared-control/base-collection allocation of 0x48, puts the collection
at allocation+10, and passes the router itself to weak construction `142ee3b30`
at collection+28 (`140797baa..140797bd2`). It installs base vtable `1459ecdd0`.
It initializes weak self using the shared controller and stores the resulting
shared collection at router+70/+78. This establishes the exact UObject type.

Node constructor `1407909b0(this, owner, widget)` initially installs the base
vtable, constructs weak owner+28 from incoming RDX, then installs node vtable and
constructs weak widget+58 from incoming R8. It returns this.

Child creation `1407a9300` allocates a node, resolves parent weak owner+28 at
`1407a9424`, and passes that exact owner as RDX to `1407909b0` at `1407a9436`.
Thus it preserves router owner identity through the tree. It also records a
weak parent at child+60/+68 and initializes weak self from its new controller.

Root factory `1407b4ac0(out, owner, widget)` allocates 0xd8 bytes, forwards its
incoming RDX/R8 unchanged to `1407909b0`, replaces vtable with `1459ece30`, and
zeros root extension +b8/+c0/+c8/+d0. It creates the shared controller, initializes
weak self, then dispatches virtual +28. Its caller `1407e4a70..1407e4a82` supplies
the router saved at `1407e4625..1407e4628` as RDX. Its widget input comes from the
widget list, distinct from that router owner. Acquisition `1407be460` searches
router+50 roots and node+70 children as established in the sibling handoff.

## Proven layout, not a new C++ class

Base: vptr+00; shared-from-this weak raw/control+08/+10; handle TArray<int32>+18
(data+18, num+20, max+24); weak UCommonUIActionRouter+28; int32+30 initialized -1.
Base extent is 0x38 in the router's 0x48 allocation after the 0x10 controller.

Node extends that base: empty array-like storage +38/+40 and +48/+50; weak
UCommonActivatableWidget+58; weak parent node+60/+68; shared children TArray+70;
another weak pair+80/+88; gate byte+90 initialized zero; 0x18-stride entries
TArray+98; another empty array-like storage+a8/+b0. Constructor initializes all
listed pairs/array header storage and no padding is claimed. Child node extent
is at least 0xb8; root factory proves root size0xd8. Do not infer source member
names or complete payload types for +38/+48/+98/+a8 from this evidence.

## Exact remaining lifecycle boundary

Gate+90 is live state. The node virtual +30 is `1407f5d50`; root overrides it
with `1407f5ec0`, which calls that base node implementation. Node setter compares
the old value, allows enabling only for a valid active widget, writes byte+90,
registers/unregisters each +98 entry through `14302d500`/`143036460`, then dispatches
virtual +30 on each shared child. It has observable ownership and callbacks.
Root adds further effects and shared ownership operations. Those complete
lifecycle methods and their callers have **not** been recovered here. A zeroed
gate or trivial setter cannot stand in for them. The destructor `140799860`
also conditionally invokes `1407ccc50`, so teardown is not just array release.

Before integrating actual route classes, recover +90 production and root
activation, +38/+48/+98/+a8 payloads/callbacks and teardown, node/root creation
and tree insertion/removal, plus the constructor's +30 producer/meaning. Exact
original C++ route class/method identifiers remain unavailable; the concrete
runtime classes are identified by original vtables and constructors, not names.
The separate owner-record fallback remains with the acquisition/registration
continuation. No whole-router completeness claim follows from this result.

## Validation

`python3 porting/frontend/common_action_route_identity_runtime/replay.py`
checks 16 bounded windows (SHA-256 per window only), three vtable slots and the
router name literal with `os.pread`. No full-image read/hash or raw image bytes
are stored. These checks validate static evidence, not an Unreal build/runtime.
Decode command: `python3 /mnt/vibrato/fortnite-work/common-action-widget-refresh-identity-1/decode.py VA_HEX:SIZE_HEX`.

Primary retains planning/integration/review; delegated bounded tasks use Luna
medium for small reads, Terra medium/high regular, Sol medium complex, always
fork_turns none. Keep searches/output economical and rotate at bounded endpoints.
