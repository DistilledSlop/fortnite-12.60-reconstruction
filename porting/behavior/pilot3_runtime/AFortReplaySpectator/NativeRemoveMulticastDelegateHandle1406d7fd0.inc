// Helper 0x1406d7fd0: TMulticastDelegate::Remove(FDelegateHandle).
// Evidence: RDX==0 returns false (FDelegateHandle::IsValid); [RCX+8] = InvocationList.Num, element stride 0x10 (TDelegateBase:
//   instance ptr + int32 size); per element vtable slot 0x30 = IDelegateInstance::GetHandle (sret in RDX) compared with the handle;
//   slot 0x38 + FMemory::Realloc(ptr,0,0) + zero size = Unbind(); 0x140685370(list, false) = CompactInvocationList(false).
// Matches MulticastDelegateBase.h TMulticastDelegateBase::Remove/RemoveDelegateInstance. Remove does not depend on the signature
// template arguments (layout is identical), so a TMulticastDelegate<void()> view is used. Callers pass different delegate members.
// CONFIDENCE: MEDIUM
return reinterpret_cast<TMulticastDelegate<void()>*>(Delegate)->Remove(Handle);
