// Body 0x1416dbf10 (thunk 0x1428b9280): info->+0x1a8 = this (CreatureManagerComponent); this->+0x100 TArray (CreatureManagerInfos) Add(info) with Num at +0x108
// Evidence: MOV [RDX+0x1a8],RCX; RBX=RCX+0x100; Num++ at +0x108; grow via 0x140690bb0 (ENGINE PATTERNS array_grow); Data[old]=RDX
// CONFIDENCE: HIGH
static_assert(sizeof(UFortCreativeCreatureManagerInfoComponent) > 0, "Info component type must be complete");
if (CreatureManagerInfoComponent == nullptr)
{
	return;
}
CreatureManagerInfoComponent->CreatureManagerComponent = this;
CreatureManagerInfos.Add(CreatureManagerInfoComponent);
