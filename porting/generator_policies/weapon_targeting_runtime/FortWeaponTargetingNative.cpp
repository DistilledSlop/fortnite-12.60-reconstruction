#include "FortWeaponTargetingNative.h"
#include "HAL/IConsoleManager.h"

// Original registrar140359080; original PE .data initializes1478df2c4 to1.
static int32 GBlockTargetingWhileUnequipped1478df2c4 = 1;
static FAutoConsoleVariableRef CVarBlockTargetingWhileUnequipped1478df2c4(
    TEXT("Weapon.BlockTargetingWhileUnequipped"), GBlockTargetingWhileUnequipped1478df2c4,
    TEXT("Allows us to block setting targeting while a weapon is unequipped."), ECVF_Default);

// Original registrar14035c680; original PE .data initializes1478df440 to1.
static int32 GAllowFirstShotAccuracyInWater1478df440 = 1;
static FAutoConsoleVariableRef CVarAllowFirstShotAccuracyInWater1478df440(
    TEXT("Weapon.AllowFirstShotAccuracyInWater"), GAllowFirstShotAccuracyInWater1478df440,
    TEXT("Sets if we allow first shot accuracy in water."), ECVF_Default);

int32 NativeBlockTargetingWhileUnequipped1478df2c4()
{
    return GBlockTargetingWhileUnequipped1478df2c4;
}

int32 NativeAllowFirstShotAccuracyInWater1478df440()
{
    return GAllowFirstShotAccuracyInWater1478df440;
}
