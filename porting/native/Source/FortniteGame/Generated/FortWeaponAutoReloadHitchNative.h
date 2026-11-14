#pragma once
#include "CoreMinimal.h"
#include "CoreGlobals.h"
#include "HAL/ThreadHeartBeat.h"
#include "HAL/ThreadManager.h"
#include "HAL/PlatformTLS.h"

// Original 1482472e4 is engine GHitchDetected; 142cd3770 is ReportHitch.
// Fail closed if the engine omits the real producer/global instead of inventing a local bool.
#if !USE_HITCH_DETECTION
#error Auto-reload native diagnostics require the engine hitch detector (USE_HITCH_DETECTION).
#endif

class FFortWeaponAutoReloadHitchScope
{
    const TCHAR* StatString;
public:
    FFortWeaponAutoReloadHitchScope()
        : StatString(GHitchDetected ? nullptr : TEXT("STAT_FortWeapon_TryAutoReload")) {}
    ~FFortWeaponAutoReloadHitchScope()
    {
        if (GHitchDetected && StatString)
        {
            // Exact engine FLightweightStatScope::ReportHitch effects, available even in STATS builds.
            const float Delta = float(FGameThreadHitchHeartBeat::Get().GetCurrentTime()
                - FGameThreadHitchHeartBeat::Get().GetFrameStartTime()) * 1000.0f;
            const bool bGameThread = FPlatformTLS::GetCurrentThreadId() == GGameThreadId;
            const FString ThreadString(bGameThread ? TEXT("GameThread")
                : FThreadManager::Get().GetThreadName(FPlatformTLS::GetCurrentThreadId()));
            const FString StackString = StatString;
            if (!bGameThread && (StackString == TEXT("STAT_EventWait")
                || StackString == TEXT("STAT_FQueuedThread_Run_WaitForWork"))) return;
            UE_LOG(LogCore, Error, TEXT("Leaving stat scope on hitch (+%8.2fms) [%s] %s"),
                Delta, *ThreadString, *StackString);
        }
    }
};
