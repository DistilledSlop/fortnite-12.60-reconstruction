// Behaviour snippet: AFortPlayerControllerAthena::ServerStartLoadingVolume_Implementation (body only; spliced into the generated member function).
// Original address: 0x1419f3360
// Decompile source: porting/generated/decomp_improved/AFortPlayerControllerAthena/ServerStartLoadingVolume_Implementation.c
// Proxy score: 65.0 (portability.json)
// Inferred: Decompile appends to VolumesLoading (TArray<AFortVolume*> at +0x35f8, reflection offset 13816): ArrayNum+1, grow if ArrayMax is exceeded, store at the old ArrayNum index. That is TArray::Add.
// Declared in generated header: yes
// CONFIDENCE: HIGH
VolumesLoading.Add(VolumeToLoad);
