// CONFIDENCE: HIGH. Complete recovered predicate.
// Original 0x1426d24d0. One timer manager lookup, with short-circuit queries.
if (AFortWeapon::NativeIsFiring708())
{
    return true;
}
const FTimerManager& TimerManager = GetWorldTimerManager();
return TimerManager.IsTimerActive(NativeFiringTimerD88)
    || TimerManager.IsTimerActive(NativeFiringTimerD90);
