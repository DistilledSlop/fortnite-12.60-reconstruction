// UCommonLoadGuard native (unreflected) member, original +0x1f0.
// Evidence: 1407f7e20 loads the qword at this+0x1f0 and returns when it is null. Only the
// object pointer is evidenced, so TSharedPtr<SLoadGuard> is the intended owning type.
// Integrate into the UCommonLoadGuard class body (porting/native is owned elsewhere). The
// SLoadGuard object itself is created by UCommonLoadGuard's RebuildWidget, which is not recovered.
TSharedPtr<SLoadGuard> NativeSlateLoadGuard;
