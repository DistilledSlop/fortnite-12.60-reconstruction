// CONFIDENCE: MEDIUM; thunk 0x142a7f620: validate (vcall 0xe58) is the return-true stub 0x1406801d0. Impl 0x14234a9c0 (FUN_14234a9c0(this, MontageToStop)) returns for a null montage, otherwise forwards the interrupt to the mesh AnimInstance (Mesh at this+0x278, AnimInstance at +0x630) via 0x143fd79c0 (not read), and, when the pawn is not locally authoritative (IsLocallyControlled via vcall 0x6b0 and Role<ROLE_Authority), raises the Blueprint event FName DAT_1481b4aa0 through ProcessEvent (0x142a774a0).
if (!MontageToStop)
{
	return;
}
UAnimInstance* AnimInst = Mesh ? Mesh->NativeRecovered_0x630 : nullptr;
if (AnimInst)
{
	AnimInst->NativeRecovered_0x143fd79c0(MontageToStop->NativeRecovered_0xe8, MontageToStop);
	if (IsLocallyControlled() && Role < ROLE_Authority)
	{
		ProcessEventRootMotionInterruptStopMontage(MontageToStop);   // FUN_142a774a0 (ProcessEvent with one object param)
	}
}
