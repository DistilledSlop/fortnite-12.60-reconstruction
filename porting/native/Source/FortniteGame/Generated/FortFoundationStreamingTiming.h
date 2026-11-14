#pragma once

#include "CoreMinimal.h"
#include <cstddef>

// Descriptive names for the original non-reflected record at foundation +0xdf8.
struct FFoundationStreamingTiming
{
    double StartedSeconds;
    double FinishedSeconds;
    double ShownSeconds;
    float ShownDistanceSquared;
    float ShownThreshold;
    uint8 bWithinShownThreshold;
    uint8 bFinished;
    uint8 bShown;
    // Implicit padding at +0x23 is deliberately not initialized.
    FColor Color;

    FFoundationStreamingTiming()
        : StartedSeconds(0.0), FinishedSeconds(0.0), ShownSeconds(0.0),
          ShownDistanceSquared(0.0f), ShownThreshold(0.0f),
          bWithinShownThreshold(0), bFinished(0), bShown(0), Color(FColor::Black)
    {}
};
static_assert(sizeof(FFoundationStreamingTiming) == 0x28, "timing record size");
static_assert(offsetof(FFoundationStreamingTiming, Color) == 0x24, "timing color offset");
static_assert(offsetof(FFoundationStreamingTiming, bShown) == 0x22, "timing flags offset");

namespace FoundationStreamingTiming
{
    // Process-shared, non-atomic count, corresponding to 0x1478c2fec.
    extern uint32 StartedCount;

    // All timing record producers must use this same helper.
    double NowSeconds();
    void Start(FFoundationStreamingTiming& Timing);
}
