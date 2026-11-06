// CONFIDENCE: HIGH
// Native141c0a7d0 guards the mesh before shared141c0a630.
UStaticMeshComponent* Mesh = StaticMeshComponent;
return Mesh ? NativeGetOrCreateMID(Mesh, ElementIndex) : nullptr;
