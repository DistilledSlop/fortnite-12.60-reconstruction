// CONFIDENCE: MEDIUM - exec 0x142779640 calls body 0x141db8ae0. Static-mesh children with the ChildComponentTag are hidden in game.
// Original addresses: 0x142779640 (exec thunk) ; 0x141db8ae0 (direct body)
USkeletalMeshComponent* PartMesh = GetSkeletalMeshForAssociatedPlayerPawnPartType();
if (PartMesh == nullptr)
{
    return;
}
TArray<USceneComponent*> Children;
PartMesh->GetChildrenComponents(/*bIncludeAllDescendants*/ true, Children);
for (USceneComponent* Child : Children)
{
    if (Child != nullptr && Child->IsA(UStaticMeshComponent::StaticClass()) && Child->ComponentTags.Contains(ChildComponentTag))
    {
        Child->SetHiddenInGame(bHideChildComponents, /*bPropagateToChildren*/ true);
    }
}
