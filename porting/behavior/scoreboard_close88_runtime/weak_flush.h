#pragma once

// Source-level recovery of 0x141e51ca0. Writer must supply the recovered
// 0x141df4ff0 Flush operation; see HANDOFF.md before integrating.
#include "Templates/SharedPointer.h"

template <typename Writer>
void ScoreboardFlushPinnedWriter(const TWeakPtr<Writer, ESPMode::NotThreadSafe>& WeakWriter)
{
    TSharedPtr<Writer, ESPMode::NotThreadSafe> WriterPtr = WeakWriter.Pin();
    if (WriterPtr.IsValid())
    {
        WriterPtr->Flush();
    }
}
