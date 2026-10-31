// Behaviour snippet: AFortPlayerStateAthena::OnRep_TeamKillScore (body only; spliced into the generated member function).
// Original address: 0x142a8de40 (exec record of the RepNotify; its tail jump reaches the body at 0x141a531d0).
// Decompile source: porting/generated/decomp_improved/AFortPlayerStateAthena/OnRep_TeamKillScore.c (body bytes: runtime dump at 0x141a531d0)
// Proxy score: none (tenth pass; not a portability record)
// Inferred: P_FINISH is the script epilogue of the exec thunk and is not ported. Two steps, in the asm order:
//   1. The owner (AActor::Owner at +0xe0, through GetOwner()) when it IsA AFortPlayerControllerAthena (the decompile's
//      AFortPlayerControllerAthena::StaticClass, 0x142a53450): the no-argument native delegate of the controller at +0x3210
//      is broadcast. The accessor 0x1419e4f10 is `lea rax,[rcx+0x3210]; ret`, and the broadcast is the global Broadcast
//      0x140684de0. No reflected property sits at +0x3210 of that class (the only property at that offset belongs to
//      AFortCharacterVehicle), so this is a native member. Name is LOW (our name, not from the binary).
//   2. TeamKillScoreNative (+0x1150) is broadcast with (this, TeamKillScore) (TeamKillScore at +0xdbc; tail call 0x1407ac640).
// Declared in generated header: yes
// CONFIDENCE: MEDIUM
AFortPlayerControllerAthena* Controller = Cast<AFortPlayerControllerAthena>(GetOwner());
if (Controller != nullptr)
{
	Controller->TeamKillScoreChangedNative.Broadcast();
}
TeamKillScoreNative.Broadcast(this, TeamKillScore);
