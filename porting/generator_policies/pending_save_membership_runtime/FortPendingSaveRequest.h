#pragma once

#include "CoreMinimal.h"

// Partial logical representation of the actual in-flight save request. This
// is not its native layout or its complete constructor/copy implementation.
// Extend this same type when the callback and payload state are recovered.
struct FFortPendingSaveRequest
{
    // Original request offsets 0x48 and 0x98. The distinct meanings and
    // mutation sites of these sets remain open.
    TSet<FString> ItemIds48;
    TSet<FString> ItemIds98;
};
