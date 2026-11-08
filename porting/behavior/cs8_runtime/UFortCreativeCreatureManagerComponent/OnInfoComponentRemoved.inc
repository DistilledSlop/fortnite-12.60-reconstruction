// Body 0x1416dbf70 (thunk 0x1428b9300): info->+0x1a8 = 0; this->+0x100 TArray compacted keeping elements != info (stable order), Num at +0x108 set to kept count
// Evidence: MOV [RDX+0x1a8],0; alternating run loop over Data[] comparing pointer to RDX; memmove 0x1458092f3 of kept runs; MOV [R13+0x108],R12D
// CONFIDENCE: MEDIUM
static_assert(sizeof(UFortCreativeCreatureManagerInfoComponent) > 0, "Info component type must be complete");
if (CreatureManagerInfoComponent == nullptr)
{
	return;
}
CreatureManagerInfoComponent->CreatureManagerComponent = nullptr;
CreatureManagerInfos.Remove(CreatureManagerInfoComponent);
