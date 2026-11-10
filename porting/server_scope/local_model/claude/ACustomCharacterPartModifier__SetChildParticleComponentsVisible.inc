// CONFIDENCE: MEDIUM - exec 0x1427796d0 calls body 0x141db8c30. FX-system children with the ChildComponentTag get their visibility set.
// Original addresses: 0x1427796d0 (exec thunk) ; 0x141db8c30 (direct body)
USkeletalMeshComponent* PartMesh = GetSkeletalMeshForAssociatedPlayerPawnPartType();
if (PartMesh == nullptr)
{
    return;
}
TArray<USceneComponent*> Children;
PartMesh->GetChildrenComponents(/*bIncludeAllDescendants*/ true, Children);
for (USceneComponent* Child : Children)
{
    if (Child != nullptr && Child->IsA(UFXSystemComponent::StaticClass()) && Child->ComponentTags.Contains(ChildComponentTag))
    {
        Child->SetVisibility(bVisible, /*bPropagateToChildren*/ true);
    }
}
