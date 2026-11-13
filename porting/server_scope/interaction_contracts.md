# Original-client interaction coverage

This is an incomplete behavioral checklist for the 12.60 original client and the
reconstructed Apollo dedicated server. It is not an exhaustive network specification.
The [declaration inventory](README.md#network-contract-coverage) is a separate layer:
908 RPC declarations and 1,961 replicated fields in the recovered metadata domain.
Neither counts nor successful sends establish correct client state.

For each flow, record the action and preconditions, incoming request/local trigger,
authority checks and state writes, per-recipient replication/RPCs, client consumer,
and visible result. Keep recovery, implementation, observation and parity verdicts
separate. Unknown means unverified; an unobserved RPC is not automatically missing
or unnecessary.

## Current evidence and gaps

Paths below are relative to `porting/`. Generated artifacts are local evidence;
source-level contracts and limitations belong in the repository. This checkpoint
does not promote historical plan tables to current runtime proof.

| Flow | Expected observation to check | Current evidence | Remaining coverage |
|---|---|---|---|
| Join and initial actor replication | Each client joins, receives its controller and the appropriate world/player actors, and stays connected. | `generated/runs/20261113-apollo-complete-two-profile-1/`: two joins and continued play after the second join. | Exact initial actor/property sequence, rejected requests, missing references, disconnect/rejoin and late-join variants. |
| Locker selection and spawn | The selected character reaches the owning client and remote observers. | Same two-profile run: distinct CID001/CID002 selections, authoritative parts binding and distinct rendered characters. | Full cosmetic/variant combinations, respawn, join-order reversal and per-recipient field receipts. |
| Player names and renames | Authoritative reads are plaintext; the original client's remote label decodes correctly. | Original codec and dedicated setter/current/old-cache lifecycle recovered; integrated server build `20261113-140804-server.log` and 19 runtime class/archive checks passed in `generated/runs/20261113-player-name-selftest-1/`. User observed garbled labels with the previous plaintext wire value. | Original-client remote-label validation; original client cover-name/cache branches remain unrecovered. |
| Warmup, aircraft and exit | Both clients agree on match phase, bus position and passenger transitions at comparable server times. | Two-profile run reached bus, forced exit and landing. Original bus tick uses server time with local correction; recovery in `generated/agents/actor-timing-audit/`. | Paired clock/location measurements; user-observed bus offset remains open. A screenshot at different times cannot establish parity. |
| Ground/airborne movement and animation | Owner input produces authoritative movement and consistent remote position, velocity, mode and animation. | Original airborne dispatch/hooks partially recovered in `generated/agents/airborne-physics-hooks/REPORT.md`; current custom movement accepts client positions. | Full custom physics and dependent hooks, authoritative velocity, correction behavior and measured owner/observer error. User-observed desync remains open. |
| Equip/unequip, pickaxe attachment and pose | Owner and observer both see the correct held item, attachment and animation through switches and initial replication. | `CurrentWeapon` skips the owner; `WeaponData` is initial-only. Original equip/attachment lifecycle partially recovered in `generated/agents/remote-pickaxe-pose/`. | Complete lifecycle integration and both join orders. User reports client 1 sees client 2's pickaxe incorrectly while client 2 sees both correctly. Owner RPC success does not cover this observer path. |
| Streaming and POI discovery | Required packages resolve for every client; entering a POI produces the appropriate per-player discovery/name/map state. | All 649 Apollo map roots staged; the two-profile retry did not repeat the prior missing-package kick. POI manager, initial discovery entries and mutations investigated in `generated/agents/hydrogen-poi-initialization/` and related reports. | Full authority manager/update and quest-persistence lifecycle, per-player discovery transitions and actual map/minimap labels. Staging alone does not implement discovery. |
| Lighthouse/world cosmetic timing | Determine whether the original behavior requires a shared phase before asserting a sync defect. | `generated/agents/hydrogen-lighthouse/REPORT.md`: placed beam uses material animation; exact shader time source not recovered. | Recover clock/phase dependency and compare at common time. Cosmetic/local-phase interpretation remains an inference. |
| Connection health and departure | Clients receive timely updates and recover or disconnect according to the actual protocol/state rules. | Earlier missing-package kick had a concrete staging cause. Quieter two-profile run still had high reported RTT and render hitches. | Exact warning-icon trigger, server/client tick delays, timeouts, loss/reordering, cleanup and reconnect. No conclusion that render cost explains all network symptoms. |

The following families also require complete contracts and scenario evidence:
inventory grants/pickups/drops and full-bar swaps; ability activation and rejection;
firing, hits, shield/health and damage cues; harvesting; building placement/edit/repair;
storm damage and elimination; DBNO/spectating/respawn where applicable; team/party
state; winner/results delivery, durable backend updates and return to lobby. Existing
implementation or historical observations must be linked to each specific assertion
before marking it verified.

## Required scenario dimensions

For each applicable flow, exercise both owners and remote observers, reverse join
order, and cover initial state versus later mutations. Include delayed asset/NetGUID
resolution, relevancy/streaming transitions, late joins, rapid repeated requests,
invalid authority/state, delayed or lost messages, and disconnect/reconnect.
Record unsupported or undefined cases explicitly instead of inventing expected
behavior. Compatibility with the original client and fidelity to unavailable original
server logic are distinct claims.

## Trace boundary

The opt-in RPC recorder and `runtime_report.py` cover selected server receive/send
stages. `invoked` means immediately before `ProcessEvent`; `queued` and `submitted`
mean local handoffs. They do not prove successful behavior, acknowledgement, client
receipt or application. Replicated properties, generic native calls, Blueprint-only
behavior and backend exchanges are not covered by this recorder. WP B1/B3 remain
incomplete until their additional tracing/ledger requirements are met.
