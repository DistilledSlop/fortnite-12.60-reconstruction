// CONFIDENCE: HIGH. Original AActor::InternalGetNetMode143f7a920.
// Keep the world lookup even in a dedicated build: the original caller observes it.
UWorld* World = GetWorld();
UNetDriver* Driver = NetDriverName == NAME_GameNetDriver
    ? (World ? World->GetNetDriver() : nullptr)
    : GEngine->FindNamedNetDriver(World, NetDriverName);
if (!Driver && World) Driver = World->DemoNetDriver;
return Driver ? Driver->GetNetMode() : NM_Standalone;
