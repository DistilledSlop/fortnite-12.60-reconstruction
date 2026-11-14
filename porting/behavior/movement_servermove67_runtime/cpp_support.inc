#pragma once

#include "WaterSubsystem.h"
#include "HAL/IConsoleManager.h"

// Original global 0x1478c51c0, registration xref 0x1402ee416 (ECVF_Default).
static int32 GFortWaterUseClientWaveTime67 = 1;
static FAutoConsoleVariableRef CVarFortWaterUseClientWaveTime67(
    TEXT("Fort.Water.UseClientWaveTime"),
    GFortWaterUseClientWaveTime67,
    TEXT("If enabled, clients send the time used for waves to the server as part of the move RPC."),
    ECVF_Default);
