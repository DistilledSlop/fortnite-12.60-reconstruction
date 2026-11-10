// CONFIDENCE: MEDIUM
// Original addresses: RPC thunk 0x1428d6f90; _Validate virtual slot 0xaa8 -> 0x1406801d0 (Stub_ReturnTrue);
// _Implementation virtual slot 0xab0 -> 0x1426b04d0 (body ported below).
// Declared parameter order (confirmed from thunk read order): BuildingLocation, BuildingRotation, Location, Rotation, InBuildingAttachmentType.
if (!ServerCreateBuildingAndSpawnDeco_Validate(BuildingLocation, BuildingRotation, Location, Rotation, InBuildingAttachmentType))
{
	RPC_ValidateFailed(TEXT("ServerCreateBuildingAndSpawnDeco_Validate"));
	return;
}

// ---- _Implementation (0x1426b04d0) ----
AFortPlayerController* InstigatorPC = GetInstigatorController();   // 0x14269e2e0
UFortDecoItemDefinition* DecoItemDef = GetDecoItemDefinition();     // virtual 0xb00 -> 0x14269bb00
if (DecoItemDef == nullptr)
{
	if (LogFortnite_Verbosity >= 3)
	{
		UE_LOG(LogFortnite, Warning, TEXT("ServerCreateBuildingAndSpawnDeco called without a valid DecoItemDef"));
	}
	return;
}
if ((DecoItemDef->bDecoFlags_b35 & 2) == 0)
{
	return;
}

// Candidate building classes for this deco item (0x142008e60 -> 0x141ff8220 on DecoItemDef+0xb40, captured, not ported).
TArray<UClass*> BuildingClasses;
GetDecoBuildingClasses(DecoItemDef, BuildingClasses);
if (BuildingClasses.Num() == 0)
{
	return;
}

// Attachment type gate: type 0,2,6,7 sets bAllowAttachedFlag, type 1 leaves it clear, anything else aborts.
bool bAllowAttachedFlag = false;
if (InBuildingAttachmentType < 8 && ((0xC5 >> InBuildingAttachmentType) & 1) != 0)
{
	bAllowAttachedFlag = true;
}
else if (InBuildingAttachmentType != 1)
{
	return;
}

// 0x141eeaa20 (side-effecting predicate that always resolves to false in this build).
bool bOverrideFlag = QueryDecoOverrideFlag(DecoItemDef, GetFortGlobals() + 0x5f8);

// Find an existing building actor to attach to (0x14237a640, 112 lines, captured, not ported).
ABuildingActor* FoundBuilding = nullptr;
FindBuildingForDecoPlacement(InstigatorPC, FoundBuilding, bAllowAttachedFlag, BuildingClasses, bOverrideFlag);
if (FoundBuilding == nullptr || !FoundBuilding->IsA(ABuildingSMActor::StaticClass()))   // 0x142752f80 = ABuildingSMActor::StaticClass
{
	return;
}

// Snap the building Z to the 384-unit grid: Z' = ((int)round(2*(Z/384 + 0.5) - 0.5) >> 1) * 384.
float SnappedZ = (float)(RoundToInt((BuildingLocation.Z * (1.0f / 384.0f) + 0.5f) * 2.0f - 0.5f) >> 1) * 384.0f;
FVector SpawnLocation(BuildingLocation.X, BuildingLocation.Y, SnappedZ);
FRotator SpawnRotation = BuildingRotation;

// Spawn the deco building (call target 0x140680300 decompiles to `return 0`, a null stub, in this build).
UWorld* World = GetWorld();                                           // virtual 0x150
AActor* SpawnedBuilding = SpawnDecoBuildingActor(InstigatorPC, FoundBuilding, SpawnLocation, SpawnRotation, /*bOverrideFlag forced to 0*/ false, World->Field540);
if (SpawnedBuilding != nullptr)
{
	// Follow-up: ServerSpawnDeco implementation (0x1426b0d50) with the original Location/Rotation and the spawned building.
	ServerSpawnDeco_Implementation(Location, Rotation, SpawnedBuilding, InBuildingAttachmentType);   // virtual 0xac0
}
