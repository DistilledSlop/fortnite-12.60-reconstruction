# Paired walking comparison trace

The adjacent patch adds opt-in `FortMovementPair` logging under
`-FortniteProbeMovementDefaults`, immediately before the existing correction
decision in `ServerMoveHandleClientError`. It preserves the decision and its
short circuit. Logging includes accepted and corrected comparisons after base
normalization; calls skipped by the existing update-delay gate remain excluded.
The logged base is the normalized comparison base, not necessarily the incoming
RPC base. Both comparison positions are rebased to zero origin for reporting.

Checkpoint `20261114-movement-acceptance213-1`, source `7b0f104a`, has 26 moving
samples with physics delta below .125 seconds; all receive corrections within
70 ms (16 full, 10 short). Steady flat server displacement equals 410 times its
physics delta. The world/wall-time ratio across the W window is .999937.
Timestamp truncation alone therefore cannot explain the corrections. Original
client physics integration time, actor dilation, exact error vectors and actual
authority flags remain unmeasured. The original client's .125 cap is not changed.

The next combined run must compare dry flat W/Shift+W separately from landscape
height transitions and blocked endpoints. Use error vector, packed modes, floor,
base, dilation and authority flags to distinguish horizontal, vertical and mode
disagreement. The trace is diagnostic support, with no original-body credit.

Engine diff whitespace and reverse-application checks passed. UE compilation and
runtime verification remain pending.

The companion `LogFoundationSpawnColumn` probe in FortAthenaMatchFlow.cpp is
gated by both existing foundation/ground-contact flags. Once the requested
foundation becomes visible, it enumerates static meshes in that exact loaded
ULevel whose calculated bounds intersect the original spawn column. It excludes
actual ALODActor instances, logs up to 128 candidates and the omitted count,
full transforms/bounds, profile/pawn response, BodySetup trace flag and shape
counts, then performs simple/complex component-targeted center-line traces.
The 42 cm footprint matches the existing fixed capsule sweep. No collision
settings change. Bounds may include empty space or aggregate instance bounds; a
center-line miss cannot exclude collision elsewhere in the capsule footprint.
Direct traces test component geometry rather than world profile filtering.
Original-client transform/contact comparison remains independent work.

Checkpoint213's existing simple and complex spawn sweeps hit only landscape
although the loaded level has 672 mesh physics states and 664 pawn-blocking
profiles. Its nearest-mesh output never identifies the actual floor component;
those counts alone cannot distinguish geometry, placement or profile defects.

The optional `-FortniteProbeFoundationColumn=X=...,Y=...,Z=...` selects a
diagnostic column independently of the spawn and existing movement sweeps. It
accepts three ordered, finite decimal coordinates; malformed input skips only
this column scan with a warning. Build `20261114-080717-server.log` passed and
was staged. Runtime253 found good flat movement comparisons within 1 cm and
repeated corrections where the client blocks near X182230, about 24 metres
from the original spawn scan. The next scan targets that measured boundary;
the blocking component is not yet identified.

Checkpoint `20261114-collision-hit259-1`, source `68afbbe2`, completed its
600-second deadline (wrapper 0, client timeout 124) without a client/server fatal
error. A hardware execution breakpoint at original Fort HandleImpact
`0x1420e2d80` captured the owned movement's blocking hit. The original
FHitResult identifies `S_Athena_Foundation_Straight66.StaticMeshComponent0`,
class FortStaticMeshActor, in `Apollo_7x12_BusPort_b`. The hit capsule centre
is `(182230.015625,-174016,-2951.885986)`, contact
`(182272.015625,-174016,-2944.233643)`, normal approximately `(-1,0,0)`.
The component world translation is `(182272.015625,-174080.078125,-3072)`,
yaw approximately 90 degrees, scale 1. Original weak-object serial/index
resolution, ownership, function bytes and vtable were checked; the capture
detached without error. Two earlier captures detached before collecting a hit
(local controller not ready; unrelated Wine exception). A recorded extra W
input revisited the boundary. This debugger run is a contact diagnosis, not
an unperturbed movement-performance acceptance run.

Original object outer names place the hit in the BusPort level. Its foundation
is `LF_7x12_Parent2_2` inside LobbyIsland_b, with local transform approximately
`(3071.969,-4608.047,-2688)`, yaw .000205 degrees. BusPort actor export 271
references component export 1385, which explicitly uses
`/Game/Packages/DS_Fortnite_Terrain_NoLOD/Elevation/S_Cave_Inner_Middle`.
The component's local `(-3072.031,2048.047,-384)` composes to the captured
world transform. Map and mesh packages are staged byte-for-byte. The mesh's
tagged BodySetup contains three convex elements (13,13,12 vertices); installed
query geometry remains unmeasured.

The opt-in server probe loads only the outer LobbyIsland_b, with no nested
foundation streaming. Its column scan cannot inspect a different unloaded
level. Zero candidates in that scan is not evidence of absent geometry in a
loaded BusPort level; a complete server level census is still needed. Original
foundation selection/initialization/streaming remains incomplete. Restore it
before claiming a collision repair; neither proximity guesses nor a recursive
first-AdditionalWorlds loader implements that lifecycle. Raw capture, object
paths and the stopped-process receipt are retained in the run directory. All
owned run/debugger processes exited and the isolated prefix was removed.
