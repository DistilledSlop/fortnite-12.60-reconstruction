// UFortPlayerPowerRatingToast::HideToast chain (translation, not integrated).
// Body: 0x144e0c9c0 (decomp-imports/Imports/HideToast__144e0c9c0.c). Wrapper: 0x142292260.
// Existing evidence: strict_toast_imports_runtime/power-hidetoast-declaration.inc and -definition.inc.

// ===== Class body: UFortPlayerPowerRatingToast =====
// Weak team-member state at +0x290 (offset from the decomp; see NOTES.md for the layout caveat).
TWeakObjectPtr<UFortTeamMemberCampaign> NativeTeamMember;
// Reflected name. Exec 0x1452a50b0 (flags 0x04080401) per existing evidence.
UFUNCTION()
void HideToast();
// OnToastFinished is reflected at +0x258 (generated header). It is not redeclared here.

// ===== Namespace scope =====
// Native multicast delegates (opaque; address-suffixed by their offset). Owner classes as in existing evidence.
struct NativeMulticast78;   // UFortTeamMemberCampaign +0x78 (non-UPROPERTY, type UNKNOWN)
struct NativeMulticast250;  // UFortRegisteredPlayerInfo +0x250 (non-UPROPERTY, type UNKNOWN)
struct NativeMulticastE8;   // UMcpProfile +0xe8 (used for CommonPublicProfile and AccountProfile)

// Native functions (address-suffixed).
bool NativeWeakPtrIsValid142ee7880(void* WeakSlot);
uint8* NativeWeakPtrGet142ee5dd0(void* WeakSlot);
// 0x140690620: MulticastRemoveAll(Multicast*, Listener). Only the pseudo-C argument order is known.
void NativeMulticastRemoveAll140690620(void* Multicast, void* Listener);
// 0x142292260 (wrapper). Pseudo-C: TeamMember in, registered player out (undefined8 in the pseudo-C;
// the UFortRegisteredPlayerInfo* type follows the existing FindToastRegisteredPlayer evidence).
UFortRegisteredPlayerInfo* NativeToastFindRegisteredPlayer142292260(UFortTeamMemberCampaign* TeamMember);
// 0x140f930b0: returns 0 when Obj+0x8 bit 0x10 is set, otherwise Obj+0x20.
uint8* NativeOuterOrNull140f930b0(uint8* Obj);
// 0x141e62c80: FindToastRegisteredPlayer. Game-instance type not confirmed (existing header says UFortGameInstance).
UFortRegisteredPlayerInfo* NativeFindToastRegisteredPlayer141e62c80(const void* GameInstance, const void* PlayerIdRef);
// 0x1406a9dc0: dynamic multicast broadcast on OnToastFinished (second argument 0 in the pseudo-C, meaning UNKNOWN).
void NativeDynamicMulticastBroadcast1406a9dc0(void* Multicast, int32 UNKNOWN_Arg2);
// Virtual slot 0x268 on the team member resolves the PlayerId. Implementation 0x142292160 (existing evidence).
// Called through the vtable in the wrapper; no direct declaration (owner class UNKNOWN in this context).

// Wrapper out-buffer (appuStack_38). Layout UNKNOWN. Its vtable is 0x145a95888 (name below).
struct NativeToastIdOut142292260;
extern void* NativeVtable145a95888;
// Wrapper cleanup. Frame locals with no visible assignment in the pseudo-C (sources UNKNOWN).
extern void* UNKNOWN_lStack_20;
extern void* UNKNOWN_plStack_28;
// Non-atomic inline refcount release at the wrapper site (same shape as NativeCtrlRelease1411d0c30).
void NativeCtrlRelease142292260(void* Ctrl);
// Free for the wrapper lStack_20 (0x142bee710; also declared in mcp_cancel_translation_runtime/declarations.inc).
void NativeFreeData142bee710(void* Data);
