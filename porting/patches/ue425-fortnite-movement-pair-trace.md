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
