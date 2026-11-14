# Battle-bus clock, replication and correction lifecycle (original 12.60 client image)

Research only, 2026-11-13. Nothing was built, run or ported for this document and no production code changed.
It explains the bus desync measured in `porting/generated/runs/20261113-movement-playerstate-acceptance-1-1`
(peak gap 60,321.5 cm, about 603 m, between two original clients).

## 1. Diagnosis

**The measured gap is well explained by the original client's clock design reacting to frames longer than
0.4 s. Both clients used software rendering, but the source of the long frames was not isolated.**

1. The original client never reads the server clock to position the bus. It adds its local world
   `DeltaSeconds` to `FlightElapsedTime` every frame and places the bus at
   `FlightStartLocation + FlightStartRotation.Vector() * FlightSpeed * FlightElapsedTime`.
2. The original world tick clamps every frame to at most 0.4 s of game time and throws the rest away. A client
   rendering slower than 2.5 frames per second therefore loses flight time on every frame.
3. The only repair is the once-per-second `ReplicatedFlightTimestamp`. Its `OnRep` measures the error, and
   `Tick` removes at most `0.33 * DeltaSeconds` of it per frame. A client can recover at most 0.33 s per second
   of its *own* game time, and nothing while its game time is itself being discarded faster than that.
4. In the run, client 1 discarded 9.45 s and client 2 discarded 15.49 s of game time during the flight. Client 1
   recovered all of it by 38.5 s. Client 2 had recovered only 8.01 s when the server destroyed the bus, so it was
   still 7.4 s (about 550 m) behind. The 603 m peak is the moment the two deficits differed most (section 4).
5. The measured deficit of each client equals its discarded time minus its bounded correction to within 0.11 s
   across the whole flight. This leaves little unexplained by that accounting, within its sampling limits.

The reconstructed server's authority path matches the reviewed original function for clock domain, timestamp cadence,
thresholds and end-of-flight handling (section 3). The reviewed clock path revealed no missing server behavior.

What is *not* established is in section 9. The main open items are a real mid-flight late join, which was
reasoned from code but not measured, and whether the original server did anything outside the aircraft class.

## 2. Evidence classes and sources

| Tag | Meaning |
|---|---|
| **[B]** | Read from the original image `/mnt/vibrato/fortnite-evidence/original-late-capture-1/image-125.runtime.bin` (base `0x140000000`): decompile, disassembly or data bytes. Address given. |
| **[R]** | Recovered reflection: field offsets and replication flags in `porting/native/Source/FortniteGame/Generated/AFortAthenaAircraft.h`. |
| **[E]** | Unreal Engine 4.25 source in `/opt/fortnite/UnrealEngine`. Corroboration only. |
| **[M]** | Measured in the run's `paired-phases.jsonl` (read-only process samples every 0.5 s, 122 valid aircraft pairs) or its `server.log`. |
| **[I]** | Inference. Not verified. |

Earlier reports this document builds on and partly supersedes, all under `porting/generated/agents/`:
`actor-timing-audit`, `bus-join-clock`, `client-clock-contract`, `flight-frame-clamp`, `aircraft-tick-recovery`,
`phase-frame-sampler`. `flight-frame-sampler` does not exist. The Tick decompile used here is
`aircraft-tick-recovery/0x1418d7fb0.c`.

## 3. The complete original lifecycle

### 3.1 Fields

| Offset | Field | Replication | Role |
|---|---|---|---|
| +0x2a8.. | `FlightInfo` (start location, rotation +0x2b4, speed +0x2c0) | replicated, no notify **[R]** | analytic path |
| +0x2d0 | `FlightStartTime` | replicated, no notify **[R]** | gate (`>= 0`) and authority clock origin |
| +0x2d4 | `FlightEndTime` | replicated, no notify **[R]** | authority end test only |
| +0x2d8 / +0x2dc | `DropStartTime` / `DropEndTime` | replicated, no notify **[R]** | authority drop-zone window |
| +0x2e0 | `bIsOutOfPhaseAircraft` | replicated **[R]** | authority: unlock on entry, self-destroy at end |
| +0x2e4 | `ReplicatedFlightTimestamp` | replicated with `OnRep_ReplicatedFlightTimestamp` **[R]** | the only correction signal |
| +0x2e8 | `FlightElapsedTime` | transient, not replicated **[R]** | drives the position on every machine |
| +0x2ec | `ClientFlightTimerDrift` | transient, not replicated **[R]** | pending correction |
| +0x2f0 | in-drop-zone flag | native, not replicated **[B]** | authority only |
| +0x2f1 | flight-finished flag | native, not replicated **[B]** | authority only |

### 3.2 Every writer of the clock fields

A scan of the image for stores to these offsets, restricted to float stores and to the aircraft code range,
finds only the following **[B]**. The scan method and its limits are in section 9.

| Field | Writers |
|---|---|
| `FlightElapsedTime` +0x2e8 | constructor `0x1418b4289` (zero); `StartFlight` `0x1418d55c6` (zero); Tick authority `0x1418d8122`; Tick client add `0x1418d814c`; Tick client correction `0x1418d81c8` |
| `ClientFlightTimerDrift` +0x2ec | `OnRep` `0x1418ce29c`; Tick client correction `0x1418d81d0` |
| `ReplicatedFlightTimestamp` +0x2e4 | `StartFlight` `0x1418d55b6`; Tick authority `0x1418d8130` |
| finished flag +0x2f1 | Tick authority `0x1418d8309`, the only store to this offset between `0x141000000` and `0x143000000` |

There is no client-side initialization of `FlightElapsedTime` from server time, no snap, no second `OnRep` and
no client writer of the finished flag.

### 3.3 Sequence

1. **Pre-spawn (warmup).** The server spawns the aircraft with all four times at -1 and elapsed at 0
   (constructor `0x1418b4050`, stores at `0x1418b4261..0x1418b4289`) **[B]**. Tick returns before any movement
   while `FlightStartTime < 0` **[B]**.
2. **Start.** `StartFlight` `0x1418d5590` writes `FlightStartTime = ReplicatedFlightTimestamp = Now`, zeroes
   elapsed and derives the end and drop times **[B]**. Clients receive the start time and timestamp together;
   `OnRep` computes drift `= elapsed - (timestamp - start) = 0`. Each client's elapsed starts counting from the
   frame it applies that bunch.
3. **Authority tick** (`Role == 3`, Tick `0x1418d8105..0x1418d8138`) **[B]**:
   `Now = GameState vtable+0x630` (server world time); `FlightElapsedTime = Now - FlightStartTime`; if
   `Now - ReplicatedFlightTimestamp > 1.0` (global `0x1478c2400`, value 1.0 in the image) then
   `ReplicatedFlightTimestamp = Now`.
4. **Client tick** (`0x1418d813d..0x1418d81d0`) **[B]**:

   ```
   FlightElapsedTime += DeltaSeconds;
   if (!replay && Smoothing != 0 && Drift != 0) {
       step = sign(Drift) * min(abs(Drift), clamp(Smoothing, 0, 1) * DeltaSeconds);
       FlightElapsedTime      -= step;
       ClientFlightTimerDrift -= step;
   }
   ```

   `Smoothing` is the global at `0x1478c2360`, value 0.33 in the image **[B]**. The server time fetched earlier in
   the function is not used on this branch.
5. **Timestamp receipt.** `OnRep_ReplicatedFlightTimestamp` `0x1418ce280` is five instructions **[B]**:
   `ClientFlightTimerDrift = FlightElapsedTime - (ReplicatedFlightTimestamp - FlightStartTime)`. It overwrites
   the pending drift; it does not accumulate. The timestamp is the server time at send, so the client's target is
   the server position minus one-way latency. That latency is never compensated.
6. **Position.** Both branches then set the actor location from the analytic formula (`0x1418d81d8` onward)
   **[B]**. A replicated transform would be overwritten on the next frame, so position replication cannot
   correct an original client.
7. **End.** Authority only: when `Now > FlightEndTime` it sets the finished flag, fires the exited and ended
   delegates and destroys itself only if out of phase **[B]**. The game mode handler `OnAircraftFlightEnded`
   (native `0x141952500`) destroys a normal aircraft. Clients never set the finished flag; the client bus stops
   only because the actor is destroyed by the server.

### 3.4 Hitches

The original world tick `0x1442e80b0` adds the raw frame delta to `RealTimeSeconds` (+0x548), then stores the
value returned by the world-settings clamp `0x1446a7c10` (virtual +0x640) into `DeltaTimeSeconds` (+0x550) and
adds it to `TimeSeconds` (+0x540). There is no carry of the remainder **[B]**, from `flight-frame-clamp`. Both
clients had `MaxUndilatedFrameTime = 0.4` and all four dilation factors at 1.0 throughout the flight **[M]**.

Consequences for the bus, with `r` = game seconds per wall second on one client:

- The bus advances at most `1.33 * r` flight seconds per wall second while drift is negative.
- `r >= 0.752` is needed just to keep pace with the server. Below that the client falls further behind even with
  correction saturated.
- A single hitch of `H` seconds loses `H - 0.4` s at once and needs `(H - 0.4) / 0.33` s of game time to recover.

### 3.5 Late joins

- **Joined before the flight starts** (the tested case). The pre-spawned aircraft is already on the client with
  start time -1. The client begins counting when it applies the start bunch. In the run both clients were within
  0.07 s and 0.14 s of the server at the first sample **[M]**. There was no late-initialization offset here, unlike
  the 4 s case in the earlier `actor-timing-audit` run, which predated the warmup sentinel fix.
- **Joined `N` seconds into the flight** **[B]** for the mechanism, **[I]** for the outcome. The actor is
  constructed with elapsed 0. The first bunch carries a timestamp different from the default, so `OnRep` runs
  **[E]** and sets drift to about `-N`. The bus then starts at the flight origin and advances at 1.33x, reaching
  the server position after about `3 * N` seconds. No original code shortens this. It was not measured.

## 4. Measurement: the run's accounting closes

Flight start is the server log line `bus flight started at 356.85` at 01:51:16.558 UTC; flight end time 398.41;
forced exit at 388.75 (31.9 s) **[M]**. Server world time advanced 55.3 s over 55.35 s of log time, so the server
clock ran at wall rate **[M]**.

Columns per client: game time discarded by the clamp since flight start (`Real - World` advance), correction
applied (`elapsed - World` advance), elapsed, deficit against the server (`wall - elapsed`), pending drift.

| Server elapsed (s) | C1 discarded | C1 corrected | C1 elapsed | C1 deficit | C1 drift | C2 discarded | C2 corrected | C2 elapsed | C2 deficit | C2 drift | Gap (m) |
|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|---:|
| 0.6 | 0.00 | 0.00 | 0.57 | 0.07 | 0.00 | 0.00 | 0.00 | 0.50 | 0.14 | 0.00 | 5 |
| 4.1 | 2.46 | 0.26 | 1.63 | 2.50 | -1.81 | 2.31 | 0.13 | 1.04 | 3.11 | -0.37 | 45 |
| 8.1 | 4.29 | 0.92 | 4.29 | 3.84 | -3.21 | 5.28 | 0.79 | 3.70 | 4.45 | -3.81 | 45 |
| 12.1 | 5.90 | 1.72 | 7.49 | 4.65 | -4.07 | 7.08 | 1.32 | 5.82 | 6.32 | -5.74 | 125 |
| 16.1 | 7.46 | 2.64 | 11.21 | 4.93 | -4.80 | 9.29 | 2.11 | 9.02 | 7.13 | -7.00 | 165 |
| 20.1 | 8.44 | 3.56 | 14.93 | 5.20 | -4.77 | 10.92 | 2.77 | 11.68 | 8.47 | -8.03 | 244 |
| 24.1 | 9.21 | 4.62 | 19.19 | 4.95 | -4.56 | 12.69 | 3.56 | 14.87 | 9.28 | -8.89 | 324 |
| 28.1 | 9.45 | 5.89 | 24.30 | 3.84 | -3.83 | 14.13 | 4.49 | 18.59 | 9.55 | -9.62 | 428 |
| 32.1 | 9.45 | 7.23 | 29.71 | 2.42 | -2.23 | 15.11 | 5.48 | 22.58 | 9.56 | -9.48 | 535 |
| 36.1 | 9.45 | 8.61 | 35.29 | 0.85 | -0.85 | 15.49 | 6.65 | 27.32 | 8.82 | -8.72 | 598 |
| 40.1 | 9.45 | 9.47 | 40.12 | 0.02 | 0.00 | 15.49 | 8.01 | 32.77 | 7.37 | -7.35 | 551 |

Readings:

- `deficit = discarded - corrected` holds at the end of the flight to 0.04 s for client 1 and 0.11 s for client 2.
  Wall time and `RealTimeSeconds` differ by up to 2.1 s in single samples during the first 10 s, which is a sample
  landing inside a long frame before the frame's delta is added, and by at most 0.51 s after that. The difference
  does not accumulate, so the loss happens at the clamp and not upstream of the world tick.
- While drift was negative each client's elapsed advanced at exactly 1.33 times its own world time. The
  correction is running at its ceiling, as the code says it must.
- The peak gap of 603.2 m is at server elapsed 33.1 s: client 1 at 31.16 s (drift -1.98), client 2 at 23.11 s
  (drift -9.79). The gap grows after both clients stop losing time because client 1 finishes catching up while
  client 2 is still limited to 0.33 s per second.
- Worst frame rate is in the first 10 s after the phase change: client 1 kept 46% of wall time, client 2 34%.
  The first sample interval contains one frame of about 2.3 s on each client.
- The forced exit happens at server elapsed 31.9 s, when client 2's own bus was 9.6 s (about 720 m) behind the
  server bus. Players leave from the server's position, not from the bus they see **[I]** for the visual effect.

**Sampler artifact.** After the server destroys the aircraft at about 41.6 s the clients' game-state array still
holds the dead actor for about 20 s, so later samples show a frozen 532 m gap. That is a stale object, not a
continuing desync. Only the 122 pairs up to destruction are valid, which is what the acceptance summary counts.

## 5. Reconstruction against the original

| Original behavior | Reconstruction | Verdict |
|---|---|---|
| Authority `Now` from game-state virtual +0x630 (server world time) | `NativeTickAuthorityFlight` uses `GameState->GetServerWorldTimeSeconds()`, which on authority is world time **[E]** | same clock domain |
| `elapsed = Now - FlightStartTime` each tick | same | match |
| Timestamp rewritten when older than 1.0 s, strict `>` | same, constant `NativeAircraftTimestampReplicationInterval` | match; timestamps in the samples step by about 1.0 s **[M]** |
| `StartFlight` writes start and timestamp together | `FortAthenaMatchFlow.cpp` `StartFlight` does, then `ForceNetUpdate` | match |
| Pre-spawn in warmup, times -1 | same | match; first-sample offsets 0.07 s and 0.14 s **[M]** |
| End: strict `Now > FlightEndTime`, delegates, game mode destroys actor | same (`AFortGameModeAthena::OnAircraftFlightEnded`) | match; both client buses froze at 41.6 s server elapsed **[M]** |
| No replicated position for the bus | none sent | match |

**No missing server behavior or incorrect clock domain was found in the reviewed clock path.**

Known gaps that are real but do not enter the bus clock:

- The server drives the authority subpath from the match-flow 0.05 s timer rather than from the actor's own Tick.
  Timestamp age is still judged against server world time, so the cadence is unchanged.
- `EnterAircraft` does not send `ClientEnterAircraft`, and the placement view helper `0x1419f5840` is not ported
  (server log lines 8732 and 8734). These affect what the client does at phase change. Whether they add to the
  client's frame cost in the first seconds is not known **[I]**.
- The generated game state lacks the Athena `OnRep_ReplicatedWorldTimeSeconds` override
  (`client-clock-contract`). It is client-side code and the clients here are original, so it has no effect on
  this result.

## 6. Protocol defect or renderer slowdown

| Observation | Protocol defect predicts | Renderer slowdown predicts | Measured |
|---|---|---|---|
| Start time, timestamp and path identical on both clients | may differ | identical | identical |
| Timestamp updates arrive about once per second on both | gaps or asymmetry | regular | regular, both |
| First-sample offset | large for the late client | small | 0.07 s and 0.14 s |
| Deficit equals discarded minus corrected | residual remains | closes | closes to 0.11 s |
| Elapsed-to-world slope while behind | not 1.33 | exactly 1.33 | 1.33 |
| A client that stops losing time converges | may not | converges at 0.33 s/s | client 1 converged at 38.5 s |

Both clients ran on `llvmpipe` through DXVK with four rasterizer threads **[M]**. The measured lost game time
and bounded correction explain the gap; the cause of the long frames was not isolated (section 9). With frames
under 0.4 s and no other source of clock error, the model predicts much smaller gaps, subject to latency and
individual hitches **[I]**.

No snapping, faster smoothing or position replication is supported by the original code, and none is proposed.
The smoothing value is a client global, so the server cannot change it either.

## 7. One decisive experiment: injected hitch on one client

**Purpose.** Show cause and effect with a known input instead of the renderer's uncontrolled one, and check the
numbers the code predicts.

**Setup.** Same server build and options as the measured run, two original clients, no code or config change.
Run `porting/tools/sample_client_phases.py PID1 PID2 --seconds 120 --interval 0.1 --output <new file>` from the
last 10 s of warmup. At server flight elapsed 12 s, suspend client 2 from outside for 5.0 s
(`kill -STOP`, then `kill -CONT`). Leave client 1 untouched. 12 s is after the phase-change hitches and leaves
enough flight to watch the recovery.

**Expected if the diagnosis is right.** Compare the paused client's changes with its pre-stop baseline and the
untouched client. The following magnitudes assume Wine includes the whole pause in the next frame delta; that
behavior has not been tested here:

1. Client 2's `RealTimeSeconds` advances by about 5.0 s while `TimeSeconds` advances by 0.4 s, adding about
   4.6 s of discarded time. Frame boundaries and unrelated hitches can change the observed increment.
2. After a fresh timestamp arrives, drift reflects the increased deficit. `OnRep` overwrites pending drift, so
   its difference from the pre-stop value also includes prior drift evolution and timestamp delivery delay.
3. While drift is negative and correction remains saturated, client 2's elapsed rises at 1.33 times its own
   world time. An incremental 4.6 s deficit takes about `4.6 / 0.33 = 13.9 s` of client game time to repay,
   assuming no further discarded time and no other changes to the baseline.
4. The incremental position deficit reaches approximately `4.6 * 7500 = 34,500 cm` (345 m) relative to the
   no-pause trajectory. Under the same assumptions it then shrinks at `0.33 * 7500 = 2,475 cm` per client game
   second. Existing drift and movement of the control client affect the observed pairwise gap.
5. The untouched client's own clock and deficit provide a control, though normal movement and independent
   hitches continue during the pause.
6. Compare `deficit - (discarded - corrected)` against the pre-stop residual and sampling uncertainty. A growing
   residual, a persistent unexpected correction slope, or irregular timestamp delivery warrants investigation;
   none alone identifies a protocol or server defect.

**Why this one.** It introduces a known external interruption and tests its effect on the accounting and recovery
slope. Background hitches, asynchronous samples and timestamp delivery remain confounders, so compare increments
and repeat if needed rather than treating the nominal numbers as exact.

**Risks.** A 5 s stop is far below the connection timeout **[E]** but this was not tried under Wine. If the
client drops, repeat with 3 s (nominally 2.6 s discarded, 195 m, 7.9 s to repay under the same assumptions).

## 8. Is there any original protection against low-frame desync

Follow-up of 2026-11-13, same image. Short answer: **not for the bus. Fortnite does have one for a different
clock, and the bus does not use it.** This is a strong negative, not a proof; the limits are in section 9.

### 8.1 What was checked and found absent

| Candidate | Result |
|---|---|
| Other virtual overrides on the aircraft | The aircraft vtable `0x145dcfd50` differs from its parent `AFortAircraft` `0x145d7cc20` in 8 of 200 slots **[B]**: destructor (+0x0), destroy cleanup `0x1418ba980` (+0x80), replicated-property registration `0x1418c5640` (+0x1b0), a reflection slot (+0x248), `BeginPlay` `0x1418bb020` (+0x328, creates two cosmetic components on non-dedicated machines), `EndPlay` `0x1418bf860` (+0x330), Tick (+0x450) and a one-call forwarder (+0x638). There is no post-net-receive, post-net-init or movement-replication override. |
| Other writers of the clock fields | A second scan over the whole image for float stores to +0x2e8 or +0x2ec that sit near references to at least two sibling fields finds only the Tick and `OnRep` stores plus constructors of unrelated classes **[B]**. |
| Smoothing strength | The value at `0x1478c2360` is the console variable `Athena.Aircraft.ClientTimeSmoothing`, help text "Amount to smooth aircraft time discrepency to correct for variable latency. 0 disables." **[B]**. Its stated purpose is latency jitter, not hitches. Tick clamps it to `[0, 1]`, so even the maximum cannot snap: the bus could then advance at most 2x local game time. |
| Other aircraft console variables | Image strings give `Athena.Aircraft.ClientTimeDebugVis`, `AircraftEnableMidpointSelectionCircle`, `AircraftEnableSpawnOnOuterCircle`, `AircraftEnableUniformDoorOpenTime`, `AircraftOffsetFromMidLineOverride`, and `Athena.bForceAircraftPhaseGC`, `Athena.DumpInventoryWhenStartingAircraft` **[B]**. None names a snap, tolerance or resync. |
| The unreflected fourth time-dilation factor (world settings +0x3e4) | Set to 1.0 by the constructor (`0x141fa4817`) and written by `0x141f05190`, whose only caller is the exec thunk of `SetEndGameTimeDilation` **[B]**. It is end-of-match slow motion, not a catch-up device. It was 1.0 in every sample of the run **[M]**. |
| `0x141975490` at phase start | Already read in `porting/server_systems/R1-warmup-and-aircraft-code.md` section 6.3: resets supply-drop spawn data. Unrelated. |
| The 60 s timer at phase start | Created at `0x14196c2b7` in `StartAircraftPhase` on handle +0x14e0. The bound member function is `0x1406801c0`, which is a bare `ret` **[B]**. It does nothing in this build. |
| Replay predicate `0x144693a30` | Returns true only when the world's demo driver (+0xf0) exists and reports playback **[B]**. False on a live client. |
| Frame clamp | Unchanged from `flight-frame-clamp`: 0.4 s, no carry. |

### 8.2 The protection that does exist, on another clock

The Athena game state keeps its own smoothed server clock, `SmoothedWorldTimeSeconds` (+0x2870) with
`SmoothedWorldTimeSecondsDrift` (+0x2874). Its update at `0x1419b15e6..0x1419b1666` is the same code shape as the
aircraft's: add `DeltaSeconds`, then remove at most `clamp(rate, 0, 1) * DeltaSeconds` of drift **[B]**. The rate
is `Athena.SmoothedWorldTimeSmoothingRate` at `0x1478c26ac`, value 0.33.

The difference is in the receive path. `OnRep_ReplicatedWorldTimeSeconds` (`0x1419a7060`) compares the drift with
`Athena.SmoothedWorldTimeDeltaTolerance` at `0x1478c26b0`, value 1.0, help text "If difference in smoothed world
time and server replicated time is >= than this, just accept server time." **[B]**. At or above 1 s it writes the
replicated time straight into the smoothed clock and clears the drift.

So the original authors wrote the same bounded smoother twice and gave only the game-state copy a hard resync.
The aircraft `OnRep` (five instructions, section 3.3) has no tolerance test and no such variable exists for it.
That is the best available evidence that the missing snap on the bus is how 12.60 shipped, and not something the
reconstruction lost.

Two consequences:

- A client-side resync of the bus is not original behavior in this build and should not be added on that claim.
- `Athena.Aircraft.ClientTimeSmoothing` is an original, client-side console variable. Raising it is a
  configuration change the original supports, but nothing found says retail did so, and it cannot exceed 1.
  It is recorded here as a fact about the binary, not as a proposal.

## 9. Remaining uncertainty

- **Mid-flight late join** is still derived from code only. The mechanism is firmer than before: the engine runs
  an on-change notify when the received value differs from the shadow state (`RepLayout.cpp` line 2964 **[E]**),
  and the timestamp's default is not a live flight time. The `3 * N` catch-up itself was not measured. Closing it
  needs one run in which a client connects after flight start.
- **Writer scans are pattern scans.** A store through a computed pointer, a block copy, or code that was still
  packed in the capture would not be seen. A live data breakpoint on +0x2e8 and +0x2ec during a flight would
  close this.
- **Original server outside the code read.** Read so far: the aircraft class, `StartAircraftPhase` and its timer,
  the flight-ended handler and the warmup path. Nothing compensates for slow clients. Server code that reaches the
  bus only through Blueprint or through the controller's aircraft component was not traced.
- **Blueprint.** `AthenaAircraft_C` could move or re-time the bus in script. Tick overwrites the location every
  frame, so a script snap would not survive one frame, but the Blueprint graph was not read.
- **Hotfix configuration.** Console variables can be set by backend-delivered configuration. Whether retail
  changed `Athena.Aircraft.ClientTimeSmoothing` for this build is unknown; the image default is 0.33 and the
  measured slope matches it.
- **Sampling.** Reads are sequential and 0.5 s apart, so per-frame deltas are inferred from totals. The table's
  server elapsed is wall time since the server log line, good to a few tens of milliseconds.
- **One run.** The earlier runs `plugin-manager-acceptance-1` and `clock-threads16-1` show the same 1.33 slope
  and discard pattern, but the closed accounting in section 4 was computed for this run only.
- **Why frames exceed 0.4 s** is out of scope by decision; the client's frame rate is taken as given.

## 10. Remote-player position drift

**This diagnosis does not explain it. It needs its own investigation.**

The bus is special: its position is never replicated, so lost game time accumulates as position error. Player
pawns are the opposite. Their position comes from the server in replicated movement on every update, so a slow
client's lost game time can delay or roughen what it shows but cannot accumulate into a standing offset **[E]**,
**[I]**. No remote-pawn positions were sampled in this run, so this is reasoning, not measurement.

What the two problems share is only the trigger. A client discarding game time also:

- simulates its own pawn slowly, and during skydiving the server currently accepts that client's position as
  authoritative, so the slowness reaches everyone;
- holds a lagging estimate of server time. `GetServerWorldTimeSeconds` on the clients ran up to 5.2 s (client 1)
  and 6.8 s (client 2) behind the server during the flight **[M]**, because the original estimator averages
  against a local clock that is losing time. Anything on the client that reads server time is late by that much.

Separate causes already on record for remote players, none of them clock problems:

- Server skydive and parachute movement (`UFortMovementComp_Character::PhysCustom`) is absent, so replicated
  velocity does not match the accepted positions (`actor-timing-audit` section 1; acceptance summary limits).
- Remote pose lifecycle is open (acceptance summary limits).

Suggested first measurement for that investigation: sample the same pawn's location and velocity on the server,
on its owning client and on the observing client at matched wall times, on the ground, where the server is
authoritative and the clamp is rarely hit.
