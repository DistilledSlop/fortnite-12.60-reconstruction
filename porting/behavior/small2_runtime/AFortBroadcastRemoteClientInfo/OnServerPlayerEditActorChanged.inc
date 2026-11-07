// Behaviour snippet: AFortBroadcastRemoteClientInfo::OnServerPlayerEditActorChanged (body only; spliced into the generated member function).
// Original address: 0x1424b5b70
// Evidence: Role (AActor, 0xf0) compared to 3 (ROLE_Authority).
// Evidence: Gate byte [RCX+0x268] is unreflected in the class own region: NativeRecoveredByte_0x268.
// Evidence: Store qword [RCX+0x270] is reflected member RemoteEditActor (Object).
// CONFIDENCE: MEDIUM
if (static_cast<uint8>(Role) == 3 && NativeRecoveredByte_0x268 != 0)
{
    RemoteEditActor = EditActor;
}
