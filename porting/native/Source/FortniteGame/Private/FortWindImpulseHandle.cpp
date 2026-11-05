#include "FFortWindImpulseHandle.h"

namespace FortWindNative
{
uint32 AllocateImpulseUID()
{
    // Original 0x148129b6c is zero-initialized storage and increments without locking.
    // Unsigned arithmetic preserves the original 32-bit wraparound.
    static uint32 NextUID = 0;
    return NextUID++;
}
}
