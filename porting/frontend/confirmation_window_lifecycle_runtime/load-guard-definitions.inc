// Original 1407f7e20, called from UCommonLoadGuard::SetIsLoading (140811170) with this = Context.
// Evidence: reads this+0x1f0 (Slate SLoadGuard), returns when it is null, otherwise stores
// bLoading into the forced-spinner byte (+0x408) and tail-calls 1408033f0, which matches
// SLoadGuard::UpdateLoadingState. SLoadGuard::SetForceShowSpinner performs the same two steps.
// Requires the NativeSlateLoadGuard member from load-guard-members.inc.
#include "SLoadGuard.h"
#include "UCommonLoadGuard.h"

void NativeSetLoadGuardLoading1407f7e20(UCommonLoadGuard* Guard, bool bLoading)
{
    if (Guard && Guard->NativeSlateLoadGuard.IsValid())
        Guard->NativeSlateLoadGuard->SetForceShowSpinner(bLoading);
}
