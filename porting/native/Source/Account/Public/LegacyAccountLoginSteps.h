#pragma once

#include "CoreMinimal.h"
#include "Delegates/Delegate.h"

// Typed counterpart of the original 0x20-byte login entries. This is a source
// model, not a binary-layout overlay. State identities must come from original
// initialization; callbacks retain the Engine delegate's actual ownership rules.
struct ACCOUNT_API FLegacyAccountLoginStep
{
    FName State;
    bool bPrimaryPlayerOnly = false;
    FSimpleDelegate Action;
};

// Original 0x1410c2f30: append even duplicate states and unbound delegates.
// The source entry must not alias the destination array. Its delegate is cloned,
// not consumed, matching the original caller-owned temporary entry.
ACCOUNT_API void AppendLegacyAccountLoginStep(TArray<FLegacyAccountLoginStep>& Steps,
    const FLegacyAccountLoginStep& Step);

// Original 0x1410d2b40: insert before/after the FIRST exact FName match. A missing
// anchor leaves the array unchanged. No deduplication or callback execution.
// The source entry must not alias the destination array.
ACCOUNT_API void InsertLegacyAccountLoginStep(TArray<FLegacyAccountLoginStep>& Steps,
    FName Anchor, bool bInsertAfter, const FLegacyAccountLoginStep& Step);
