// Generated from bot_inventory_queries_runtime.
#include "PhoebeInfiniteAmmoCVar.h"
#include "HAL/IConsoleManager.h"
// Original initialized .data byte 0x1478c29a1; bool reference registrar 0x14027bd20.
static bool GOriginalPhoebeInfiniteAmmo1478c29a1 = true;
static FAutoConsoleVariableRef CVarOriginalPhoebeInfiniteAmmo1478c29a1(
    TEXT("Phoebe.InfiniteAmmo"),
    GOriginalPhoebeInfiniteAmmo1478c29a1,
    TEXT("Set true so bots have infinite ammo."),
    ECVF_Default);
// Original reader 0x141a49680.
bool NativePhoebeInfiniteAmmo1478c29a1()
{
    return GOriginalPhoebeInfiniteAmmo1478c29a1;
}
