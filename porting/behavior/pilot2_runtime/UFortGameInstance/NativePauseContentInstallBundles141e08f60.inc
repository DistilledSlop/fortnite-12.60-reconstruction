// Helper 0x141e08f60 (caller UFortGameInstance::PauseContentInstall, RDX = 3): for each live element of the sparse array at this+0x6d0 call
// IInstallBundleManager::PauseUpdateContent on the element's bundle-name arrays.
// Evidence: [this+0x6c0] is an unreflected pointer (lookup_offset: none) whose vtbl[0x68] is the PauseUpdateContent slot of
// /opt/fortnite/UnrealEngine IInstallBundleManager (slot order: ~, HasBundleSource, PushInit, Pop x3, GetInitState,
// RequestUpdateContent, GetContentState, CancelAllGetContentStateRequestsForTag, RequestRemoveContentOnNextInit,
// CancelRequestRemoveContentOnNextInit, CancelUpdateContent=0x60, PauseUpdateContent=0x68).
// The 16-byte local passed in RDX is TArrayView<const FName>{[elem+0x120] data, [elem+0x128] num} (second one at +0x130/+0x138).
// No extra argument (R8 untouched).
// Element array = TSparseArray with 0x1a8-byte elements: Data ptr @+0, allocation TBitArray<TInlineAllocator<4>> @+0x10
// (inline 4 words, secondary pointer @+0x20, NumBits @+0x28); 0x140687d00 is the set-bit iterator advance (unvisited bit cleared).
// Bit 0 of Flags selects the first array, bit 1 the second.
uint8* const InstanceBytes = reinterpret_cast<uint8*>(GameInstance);
if (*reinterpret_cast<void**>(InstanceBytes + 0x6c0) == nullptr)
{
	return;
}
const uint8* const SparseBytes = InstanceBytes + 0x6d0;
uint8* const ElementStorage = *reinterpret_cast<uint8* const*>(SparseBytes);
const uint32* const InlineBits = reinterpret_cast<const uint32*>(SparseBytes + 0x10);
const uint32* const SecondaryBits = *reinterpret_cast<const uint32* const*>(SparseBytes + 0x20);
const uint32* const AllocationBits = (SecondaryBits != nullptr) ? SecondaryBits : InlineBits;
const int32 AllocatedBitCount = *reinterpret_cast<const int32*>(SparseBytes + 0x28);
for (int32 SlotIndex = 0; SlotIndex < AllocatedBitCount; ++SlotIndex)
{
	if ((AllocationBits[SlotIndex >> 5] & (1u << (SlotIndex & 31))) == 0)
	{
		continue;
	}
	const uint8* const SlotBytes = ElementStorage + static_cast<SIZE_T>(SlotIndex) * 0x1a8;
	IInstallBundleManager* const InstallManager = *reinterpret_cast<IInstallBundleManager**>(InstanceBytes + 0x6c0);
	if ((Flags & 0x1) != 0)
	{
		const FName* const NameData = *reinterpret_cast<const FName* const*>(SlotBytes + 0x120);
		const int32 NameCount = *reinterpret_cast<const int32*>(SlotBytes + 0x128);
		const TArrayView<const FName> BundleView(NameData, NameCount);
		InstallManager->PauseUpdateContent(BundleView);
	}
	if ((Flags & 0x2) != 0)
	{
		const FName* const NameData = *reinterpret_cast<const FName* const*>(SlotBytes + 0x130);
		const int32 NameCount = *reinterpret_cast<const int32*>(SlotBytes + 0x138);
		const TArrayView<const FName> BundleView(NameData, NameCount);
		InstallManager->PauseUpdateContent(BundleView);
	}
}
// CONFIDENCE: MEDIUM
