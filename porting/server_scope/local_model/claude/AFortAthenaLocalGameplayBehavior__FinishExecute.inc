// CONFIDENCE: HIGH for control flow; MEDIUM for the helper names used for the BT task release.
// Proposed (unreflected) names: CachedTask->TaskOwnerComponent is the task +0x20 owner (class not identified); FindBehaviorTreeNodeForTask is deps/AFortAthenaLocalGameplayBehavior_FindBehaviorTreeNodeForTask.json.
// Original address: 0x1427daf10 (exec thunk; P_FINISH ignored). Callees: 0x1418a0060 (task release), 0x143f84be0 (SetNetDormancy), 0x1449c6900 path.
// Behaviour: if a cached behaviour task exists, restore net dormancy to DORM_DormantAll when the actor was
// awakened for execution (bReplicates && bNeedToAwakeDuringExecution), then finish the task on the owning
// UBehaviorTreeComponent (only when the task owner IsA BehaviorTreeComponent), and clear CachedTask.
if (CachedTask != nullptr)
{
    if (bReplicates && bNeedToAwakeDuringExecution)
    {
        SetNetDormancy(ENetDormancy::DORM_DormantAll);
    }

    // FUN_1418a0060: owner = CachedTask->Owner (+0x20); if it IsA UBehaviorTreeComponent, finish the matching node.
    if (UObject* TaskOwner = CachedTask->TaskOwnerComponent)
    {
        if (TaskOwner->IsA(UBehaviorTreeComponent::StaticClass()))
        {
            UBTNode* Node = FindBehaviorTreeNodeForTask(CachedTask);   // FUN_1449c66c0
            Cast<UBehaviorTreeComponent>(TaskOwner)->OnTaskFinished(Node, EBTNodeResult::Succeeded); // FUN_1449cfb30(Owner, Node, 0)
        }
    }
}
CachedTask = nullptr;
