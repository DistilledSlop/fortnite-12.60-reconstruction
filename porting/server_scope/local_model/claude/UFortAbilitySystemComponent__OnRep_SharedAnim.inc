// CONFIDENCE: MEDIUM. Replication callback for the shared animation payload. Re-applies the payload, then, for an alive AFortAIPawn owner, maps the replicated animation state onto the pawn's anim-state flag bits (and nudges the pawn's animation component).
// Original addresses: exec 0x142781830; apply helper 0x14160c6f0; liveness helper 0x140718890; AFortAIPawn class 0x1427a1790.
ApplySharedAnim(SharedAnim);   // helper 0x14160c6f0 (not recovered in full)

if (!IsAbilityActorInfoPendingKill(AbilityActorInfo))   // helper 0x140718890: avatar weak-pointer validity, then virtual 0x6a8 check
{
	AFortAIPawn* OwnerPawn = Cast<AFortAIPawn>(AnimatedPawn);
	if (OwnerPawn)
	{
		if ((OwnerPawn->AnimStateFlags & 0x10) != 0 && OwnerPawn->AnimComponent && OwnerPawn->AnimComponent->CanRunBlend())   // virtual 0x540 predicate; 0x528 call with (0.5f, false, false)
		{
			OwnerPawn->AnimComponent->SetBlendWeight(0.5f, false, false);
		}

		switch (SharedAnim.State)
		{
		case 3:
			OwnerPawn->AnimStateFlags = (OwnerPawn->AnimStateFlags & 0xc3) | 0x01;
			return;
		case 4:
			OwnerPawn->AnimStateFlags = (OwnerPawn->AnimStateFlags & 0xef) | 0x02;
			return;
		case 5:
		case 6:
			OwnerPawn->AnimStateFlags = (OwnerPawn->AnimStateFlags & 0xca) | 0x08;
			return;
		case 8:
			OwnerPawn->AnimStateFlags = (OwnerPawn->AnimStateFlags & 0xd2) | 0x10;
			return;
		case 10:
			OwnerPawn->AnimStateFlags = (OwnerPawn->AnimStateFlags & 0xe2) | 0x20;
			return;
		case 11:
			break;
		default:
			OwnerPawn->AnimStateFlags &= 0xc2;
			break;
		}
	}
}
