// CONFIDENCE: high; the native target resets the report to its native defaults and clears the notification flag.
{
	PlayerDeathReport.ServerTimeForRespawn = 0.0f;
	PlayerDeathReport.ServerTimeForResurrect = 0.0f;
	PlayerDeathReport.LethalDamage = 0.0f;
	PlayerDeathReport.KillerPlayerState = nullptr;
	PlayerDeathReport.KillerPawn = nullptr;
	PlayerDeathReport.KillerHealthPercent = 0.0f;
	PlayerDeathReport.KillerShieldPercent = 0.0f;
	PlayerDeathReport.KillerWeapon = nullptr;
	PlayerDeathReport.DamageCauser = nullptr;
	PlayerDeathReport.bDroppedBackpack = false;
	PlayerDeathReport.bNotifyUI = true;
	PlayerDeathReport.Tags.Reset();
	PlayerDeathReport.ViewLocationAtTimeOfDeath = FVector::ZeroVector;
	PlayerDeathReport.ViewRotationAtTimeOfDeath = FRotator::ZeroRotator;
	bClientNotifiedOfPawnDied = false;
}
