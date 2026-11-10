// Behaviour snippet: AFortPawn::ServerInternalEquipWeapon (body only; spliced into the generated member function).
// Original address: 0x142a1f670
// Decompile source: porting/generated/native134-virtual-bulk/0x142a1f670.c
// Inferred: Standard native Server-RPC-with-validation thunk. The FFrame reads the single Object param Weap (AFortWeapon*, reflection offset 0, class 0x142b3de80). The body calls the virtual _Validate (vtable slot 0x8e8); if it returns false it calls RPC_ValidateFailed with the function name, otherwise it calls the virtual _Implementation (vtable slot 0x8f0). This is exactly the generated Server/WithValidation dispatch, so the spliced body mirrors it.
// Declared in generated header: yes (AFortPawn.h lines 1012-1015: UFUNCTION(Server, Reliable, WithValidation) void ServerInternalEquipWeapon(AFortWeapon* Weap); + _Implementation + _Validate)
// CONFIDENCE: HIGH
if (ServerInternalEquipWeapon_Validate(Weap))
{
	ServerInternalEquipWeapon_Implementation(Weap);
}
else
{
	RPC_ValidateFailed(TEXT("ServerInternalEquipWeapon_Validate"));
}
