// Behaviour snippet: GetFortGameStateAthena (kind static_helper: a static free function in the class .cpp; body only).
// Original address: 0x1419d3ff0 (queue row 4 of porting/behavior/CALLEE_PORT_QUEUE.md). Name table: helpers.json, eighth pass.
// Decompile source: porting/generated/callee_probe/blocking/0x1419d3ff0.c (disassembly in the .asm beside it)
// Proxy score: none (queue row 4; not a portability record)
// Inferred: Cast<AFortGameStateAthena>(WorldContextObject->GetWorld()->GameState). GetWorld is the UObject virtual at
//   vtable +0x150 (UObject::GetWorld() const), GameState is UWorld +0x130 (read through the public UWorld::GetGameState(),
//   which returns that field; the field itself is private in UE 4.25), and the cast is an IsA test against
//   AFortGameStateAthena::StaticClass (0x14290f690). A null world, a null game state or a game state of another class
//   gives null. The decompile passes the object in the first argument, which the Eighth pass names WorldContextObject.
// Declared in generated header: no (static_helper: the .cpp defines it, see porting/behavior/README.md, "static_helper")
// CONFIDENCE: HIGH
UWorld* World = WorldContextObject->GetWorld();
if (World == nullptr)
{
	return nullptr;
}
AGameStateBase* GameState = World->GetGameState();
if (GameState == nullptr || !GameState->IsA(AFortGameStateAthena::StaticClass()))
{
	return nullptr;
}
return static_cast<AFortGameStateAthena*>(GameState);
