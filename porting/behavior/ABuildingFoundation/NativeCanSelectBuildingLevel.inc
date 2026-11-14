// CONFIDENCE: high; original client 0x141bebba0, build/runtime validation pending.
// Resolve first even for world tiles; the original client's later direct callee always returns false.
AFortWorldManager* Manager = AFortWorldManager::NativeResolveWorldManager(GetWorld());
return IsA<AWorldTileFoundation>() || (Manager && Manager->bStreamInBuildings);
