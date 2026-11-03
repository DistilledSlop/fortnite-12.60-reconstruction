// CONFIDENCE: high; native 0x141a23e80 visits the six character part meshes.
bool bFoundComponent = false;
for (USkeletalMeshComponentBudgeted* Component : CharacterPartSkeletalMeshComponents)
{
    if (Component)
    {
        Component->SetVisibility(bVisible, true);
        bFoundComponent = true;
    }
}
return bFoundComponent;
