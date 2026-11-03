# Pawn sound forwarding, batch71

Integrated AFortPawn::PlaySound_Implementation, native142341850, into the
canonical source manifest. This was a reflection-only generated stub. The body
calls UGameplayStatics::PlaySoundAtLocation with this, Sound, actor location,
zero rotation, VolumeMultiplier, PitchMultiplier, zero start time and three
null optional objects. This restores the forwarding behavior; runtime audio
and complete engine/game parity are not proved by this source recovery.

The prior skipped manifest signature reversed the Sound/Volume/Pitch order.
Reflection offsets are Sound0, Volume8, Pitch12; exec142a1f0d0 loads SoundRDX,
VolumeXMM2, PitchXMM3 and dispatches virtual8f8. Original pawn vtable146115880
+8f8 points to142341850. That body leaves SoundRDX intact, copies root+130's
component location, falls back to the original zero vector, and forwards both
scalars unchanged in stack slots20/28 to144224f50. Its decompiler prototype
mistakenly calls one overwritten SIMD temporary PitchMultiplier; assembly
shows XMM3 stays intact. Engine reflected PlaySoundAtLocation exec144711920
calls the same native144224f50. Original globals148218630..648 are six zeros.
Local UE Actor.h GetActorLocation reads RootComponent->GetComponentLocation()
or ZeroVector, matching the native operations. No extra world/sound guard added.

Full body, exec, engine exec, zero constants and vtable pointer windows are
byte-pinned in native-evidence.json through the aggregate source policy. Both
reflected function contracts are frozen in pawn_cues66_runtime/native-contracts.
No generated code was edited by hand. Explicit include is engine GameplayStatics;
class/struct types continue to use normal generated inference.

Command: python3 /tmp/promote-native-batch71.py. Generation first rejected the
new snippet for missing CONFIDENCE comment; corrected and rerun accepted2,255,
rejected0, one generated source changed, none removed. Initial log retained as
porting/generated/gameplay-bulk-integration-71/promotion-attempt1.log; final
promotion.log/result.json/reports show the accepted source. This is generation,
not a UE build or test. No compiler/runtime/tests ran. Canonical2,075+180fields;
last tested2,040, 35 canonical+4 manual bodies pending, next tests3,040. Root
retains sole integration/build/commit ownership and economical bounded reads.
