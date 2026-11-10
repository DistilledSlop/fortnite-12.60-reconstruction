// CONFIDENCE: HIGH for control flow (checked via disassembly of impl 0x14234a910); validate (vcall 0xf88) is the return-true stub 0x1406801d0. Impl picks the weapon's unable-to-perform montage override (weapon+0xb10) over the pawn default (this+0x1420), and plays it unless the ability system component reports a blocking ability (FUN_14070c910 nonzero).
UAnimMontage* Montage = UnableToPerformActionMontage;
if (CurrentWeapon && CurrentWeapon->NativeRecovered_0xb10)
{
	Montage = CurrentWeapon->NativeRecovered_0xb10;
}
if (Montage)
{
	const bool bBlocked = AbilitySystemComponent && BlocksUnableMontage(AbilitySystemComponent);
	if (!bBlocked)
	{
		PlayAnimMontage(Montage, 1.0f, NAME_None);
	}
}
