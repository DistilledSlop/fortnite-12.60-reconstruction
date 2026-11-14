// CONFIDENCE: HIGH. Original 0x1419afaa0.
if (UStaticMeshComponent* Mesh = GetStaticMeshComponent())
    Mesh->SetVisibility(bVisible, true);
MarkComponentsRenderStateDirty();
