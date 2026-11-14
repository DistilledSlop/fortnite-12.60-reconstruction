#include "FortFoundationStreamingTiming.h"
#include "SharedFortCsv.h"
#include "HAL/PlatformTime.h"
#include <cmath>

namespace FoundationStreamingTiming
{
uint32 StartedCount = 0;

double NowSeconds()
{
#if PLATFORM_WINDOWS
    // Windows Seconds already includes the original 2^24 offset.
    return FPlatformTime::Seconds();
#elif PLATFORM_UNIX
    // Unix Seconds uses the engine-selected ClockSource and omits that offset.
    return FPlatformTime::Seconds() + 16777216.0;
#else
#error Select and validate a consistent timing domain for this platform.
#endif
}

void Start(FFoundationStreamingTiming& Timing)
{
    // The original UCOMISD/JNZ also enters this branch for unordered values.
    if (Timing.StartedSeconds == 0.0 || std::isnan(Timing.StartedSeconds))
    {
#if CSV_PROFILER
        FCsvProfiler::RecordCustomStat("BuildingStartedStreaming", CSV_CATEGORY_INDEX(Fort),
            int32(1), ECsvCustomStatOp::Accumulate);
#endif
        Timing.StartedSeconds = NowSeconds();
        ++StartedCount;
    }
    Timing.Color = FColor::Yellow;
}
}
