// CONFIDENCE: MEDIUM
// Original address: 0x142a7b400 (exec thunk, no parameters). Capture: captures/13799d83931e48b19e2b5ccf2a1528b5/0x142a7b400.c
// FUN_140aae720 = multicast delegate broadcast (read in full). FUN_143f6c200 = destroy actor (read in full).
if (BalloonRope != nullptr)
{
	BalloonRopeDelegate.Broadcast(this, 0);   // DAT_1478de5a8, multicast; listeners return a bool, and any false compacts the list
	BalloonRope->DetachAndRelease(true);      // vslot 0x2f8, unnamed; name proposed
	DestroyActorChecked(BalloonRope, false, true);   // FUN_143f6c200: World->DestroyActor(actor, bNetForce=false, bShouldModifyLevel=true)
	BalloonRope = nullptr;
}
