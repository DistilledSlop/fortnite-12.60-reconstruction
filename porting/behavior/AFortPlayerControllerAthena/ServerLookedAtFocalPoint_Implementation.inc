// Behaviour snippet: AFortPlayerControllerAthena::ServerLookedAtFocalPoint_Implementation (body only; spliced into the generated member function).
// Original address: 0x1419f1f70
// Decompile source: porting/generated/decomp_improved/AFortPlayerControllerAthena/ServerLookedAtFocalPoint_Implementation.c
// Proxy score: 70.1 (portability.json)
// Inferred: Decompile tests FocalPoint (+0x2b40) and broadcasts OnUsingFocalPointChanged (+0x2b60). The asm (body 0x1419f1f70) builds the block as {this @0, FocalPoint @8, true @0x10}; by name that is (bUsingFocalPoint=true, FocalPointActor=FocalPoint, FocalPointUser=this), using the same reversed-order rule as above.
// Declared in generated header: yes
// CONFIDENCE: MEDIUM
if (FocalPoint != nullptr)
{
	OnUsingFocalPointChanged.Broadcast(true, FocalPoint, this);
}
