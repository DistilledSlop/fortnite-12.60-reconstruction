#pragma once

#include "CommonActionBindingData.h"

// Candidate operation bodies for the original route class, whose identity and
// construction are not yet recovered. These functions OWN NO STATE. Callers must
// pass the actual route's +18 array and +08/+10 weak self, never parallel storage.
// NotifyIfActive is the mandatory original sequence: fresh virtual +18; if true,
// resolve weak UObject +28, then broadcast its +a8 delegate (no null fallback).
// Keeping that sequence in a required hook avoids inventing a route class or a
// dummy router/provider while the real owner and vtable are still unresolved.
namespace CommonActionRouteBindingOperations
{
    // 1407a90c0. Record must be the existing global registry record, freshly
    // obtained by the caller. It is not accessed again after notification.
    template <typename TNotifyIfActive>
    void Associate(TArray<int32>& Handles,
        const TWeakPtr<FCommonActionBindingRoute>& WeakSelf,
        FCommonActionBindingRecord& Record, TNotifyIfActive&& NotifyIfActive)
    {
        if (Handles.Contains(Record.Id)) return;
        Handles.Add(Record.Id);
        {
            // The original pins weak self before replacing the record's weak
            // reference and releases this temporary BEFORE the virtual query.
            const auto Self = WeakSelf.Pin();
            Record.Route = Self;
        }
        NotifyIfActive();
    }

    // 1407f0ab0 / stable all-match removal 1407ef330. Does not shrink capacity.
    template <typename TNotifyIfActive>
    void Disassociate(TArray<int32>& Handles, int32 Id,
        TNotifyIfActive&& NotifyIfActive)
    {
        if (Handles.Remove(Id) <= 0) return;
        if (FCommonActionBindingRecord* Record = CommonActionBindingData::Find(Id))
        {
            // No "still refers to this route" check exists in the original.
            Record->Route.Reset();
        }
        // Notification is still required when the registry record is absent.
        NotifyIfActive();
    }

    // 1408018c0. The hook must invoke 1407f0ab0's recovered body on the pinned
    // actual route. It has no default, and must not also remove registry data.
    template <typename TRemoveFromRoute>
    void Unregister(int32& Handle, TRemoveFromRoute&& RemoveFromRoute)
    {
        if (FCommonActionBindingRecord* Record = CommonActionBindingData::Find(Handle))
        {
            if (Record->Route.IsValid())
            {
                const auto Route = Record->Route.Pin();
                RemoveFromRoute(*Route, Handle);
            }
        }
        // Release the temporary strong route before removing registry data.
        // Reentrant notification/destruction can change Handle: read it again.
        CommonActionBindingData::Remove(Handle);
        Handle = -1;
    }
}
