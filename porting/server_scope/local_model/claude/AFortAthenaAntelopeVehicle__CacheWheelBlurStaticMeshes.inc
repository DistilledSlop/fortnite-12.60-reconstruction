// CONFIDENCE: MEDIUM. Original exec 0x1427cf900; native body 0x1418bc3e0 (inlined).
// Thunk reads the four wheel meshes in declaration order (InBL, InBR, InFL, InFR); the reflection list is in reverse order.
// The front-right and back-left meshes get a dynamic material instance on element 0, using the current material as the source.
// Vtable slot 0x548 resolves to UStaticMeshComponent::GetMaterial and slot 0x570 to UPrimitiveComponent::CreateDynamicMaterialInstance.
WheelBlur_FR = InFR;
WheelBlur_FL = InFL;
NativeRecoveredObject_0x1108 = InBL;
NativeRecoveredObject_0x1110 = InBR;

if (InFR)
{
    NativeRecoveredObject_0x10f8 = InFR->CreateDynamicMaterialInstance(0, InFR->GetMaterial(0), NAME_None);
}

if (InBL)
{
    NativeRecoveredObject_0x1100 = InBL->CreateDynamicMaterialInstance(0, InBL->GetMaterial(0), NAME_None);
}
