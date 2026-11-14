# Weapon charge state

The three complete original algorithms are `142698da0` (lower/stop charge),
`1426b21a0` (state transition) and `1426933f0` (state-change events). Captures,
hashes and exact engine equivalents are in `charge-state-evidence.json`;
static FName initializer and UTF-16 event identities are in
`charge-state-event-names.json`.

The local native state and sequence are one shared uint16 at original `906`.
They are separate from reflected replicated int16 ChargeStatusPack at `904`.
Transition writes state and wrapped sequence, invokes Blueprint callbacks,
queries the pawn and optionally sends ServerSetChargeState, then copies the
current local pack after potential reentry and invokes virtual ForceNetUpdate.
The unreflected native bit `900/40` is separate shared storage; fresh UObject
allocation supplies zero initialization to these native members.

The authority scope uses the actual FScopedPredictionWindow(ASC,true) and
RemoveGameplayCue on GameplayCue.Weapons.ChargeUp, including the original
PendingKill-only check. ChargeTime's `COMISS zero,value; JC` takes positive and
unordered values. ChargeDownTime's `COMISS value,zero; JBE` selects state zero
for nonpositive and unordered values; positive selects state four. It is the
actual stat ChargeDownTime, not MaxChargeTime.

Independent assembly/source review confirmed both float directions, sequence
wrap, events, engine calls and callback/RPC/copy order. Complete invoked bodies
do not establish the full weapon state producer lifecycle or runtime parity.
