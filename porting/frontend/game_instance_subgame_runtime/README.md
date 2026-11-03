# Game instance subgame transition: bounded recovery

This is an incomplete, unregistered source candidate. Do not integrate it yet:
the byte-7bb writers and the setter's transitive operations have not
been implemented. `policy.apply()`
rejects promotion. The source fragments supply the getter, complete immediate setter sequence,
delegate signatures and broadcast operation. Their typed external contracts
must be implemented before they can be integrated.
The aliases describe addresses and do not claim original symbol names.

`141e5f160` returns the byte at GI+828. Its enum is the reflected `ESubGame`
(`Campaign=0`, `Athena=1`, `Invalid=Count=2`); the director's branch for byte
2 chooses Entryway, which does not change the enum identity.

`141e7ec30` accepts an `ESubGame` by value. It emits verbosity-7 diagnostics,
then returns when the old value equals the new. On change it stores the byte,
gets a global service via `141df62a0` and invokes `141deca00(service, value)`,
gets an optional GI-associated object via `141856620` and, if non-null, calls
its virtual slot 298 with a pointer to the one-byte argument. It calls
`141e78990(this, false)`, broadcasts the native one-parameter multicast at
GI+830, then broadcasts the reflected `OnSubGameChangedBP` at GI+860. Native
broadcast uses a reverse traversal, nested invocation lock, and deferred
compaction; Unreal's native multicast delegate is required, not a hand-written
callback list. The global service transition is substantial: `141deca00`
already checks its own byte 6cc and continues through a large body. It cannot
be represented by a no-op or a direct assignment.

A second writer starts at `141e648aa`: it parses text into a byte initialized
as 2, accepts only values below 2, and inlines the same transition sequence.
Constructor `141e48c70` sets byte 828 to 2 (`ESubGame::Invalid`), zeros
the native delegate storage at 830 and 848, and sets dword 7b8 to 1, giving
byte 7bb the initial value zero. A complete source join must retain the other
writer and all writers of the manager-change gate at GI+7bb.
Scanning the bounded 141d00000..142000000 window for the literal 7bb displacement
found no genuine matching GI access; that does not rule out packed or wider
writes, other code ranges, or constructor initialization.

Athena root 14514ace0 independently calls `140684de0` on GI+848 after the
setter. Its body is the native zero-argument multicast broadcast, with reverse
traversal, invocation lock, and deferred compaction. It is distinct from the
subgame-changed multicast at 830 and the reflected multicast at 860. The
address-named `NativeBroadcastSubGameSelection848` is a semantic alias only.
The identity, producers and bindings of that delegate remain to be recovered.
The native member positions here are evidence, not instructions to perform raw
offset access. Preserve existing class reflection and proper C++ object layout.

`policy.validate_original` checks only bounded instruction windows from the
specified original image. It and fragment loading validate provenance, not a
build or startup behavior. No generated, Engine, build, or staged files were
changed.

The subsystem getter requests `UFortUIDirectorSubsystemBase` from the GI
subsystem collection. The installed `UFortUIDirector` vtable slot 298 is
`145165540`, a tail jump to the already recovered manager selection body
`145176990`. The setter passes the address of the input enum, but that
implementation ignores it; source reference constness remains inferred.

`141e78990(this, false)` snapshots the GI `RegisteredPlayers` array at 710
into weak pointers before iterating, rejects invalid or unreachable objects,
then calls `141e79020` with the registered player's members at 38 and 60.
It then calls `141f70680` on the original player with the bool argument and a
temporary output array, whose contents are destroyed. `141e79020` looks up a
registered player by identity, applies world and player gates, may create and
append a new registered player, and updates two of its fields. These callees
remain real typed dependencies; neither refresh nor registration is a no-op.

The setter logging category storage is at `1481348e4`, with verbosity byte at
`1481348e0`. Its alias in the source is an integration contract, not a new
independent logging category. The source fragment requires the existing
reflected `OnSubGameChangedBP` and ESubGame type. It must be joined to the
original subsystem virtual hierarchy; merely adding an unrelated method would
change dispatch.
