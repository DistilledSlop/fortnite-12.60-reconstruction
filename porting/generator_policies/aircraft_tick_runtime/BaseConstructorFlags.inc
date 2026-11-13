// Exact named effects recovered in AFortAircraft ctor 0x141760af0.
// Fragment: parent ctor also creates/assigns its root and initializes camera/exit.
PrimaryActorTick.bCanEverTick = true;          // +0x32 |= 0x06
PrimaryActorTick.bStartWithTickEnabled = true;
bReplicates = true;                           // +0x5b |= 0x01
PrimaryActorTick.TickGroup = TG_PrePhysics;    // +0x30 = 0
SetReplicateMovement(false);                  // 0x143f85010: +0x58 bit 0x20
bAlwaysRelevant = true;                       // +0x58 |= 0x10
SetCanBeDamaged(false);                       // 0x143f84450: +0x5a bit 0x01
SetActorEnableCollision(false);               // 0x143f83090: +0x5c bit 0x04
// No bAllowTickOnDedicatedServer override: preserve the engine's inherited default.
