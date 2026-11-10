// CONFIDENCE: HIGH
// Original address: 0x142a5a2b0 (RPC thunk). FFrame read order: PhysicsObject (+0), ClientPing (+8), ClientBallState (+16).
// FFrame bytecode skip (Code += Code != 0) is script-VM bookkeeping and is not ported.
// _Validate target 0x1406801d0 is Stub_ReturnTrue; _Implementation target 0x1406801c0 is Stub_Empty.
if (!SendClientPhysicsBallStateToServer_Validate(PhysicsObject, ClientPing, ClientBallState))
{
	RPC_ValidateFailed(TEXT("SendClientPhysicsBallStateToServer_Validate"));
	return;
}
SendClientPhysicsBallStateToServer_Implementation(PhysicsObject, ClientPing, ClientBallState);
