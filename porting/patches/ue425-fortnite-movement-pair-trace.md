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
