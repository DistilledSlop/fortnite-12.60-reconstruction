#pragma once
class UObject;
class UCommonInputManager;
class IFortUIManagerInterface;

// Layout evidence: the caller of original 14514eda0 (root-native-scan.asm, 145147574)
// reads entry 0 as the manager object and entry 8 as the IFortUIManagerInterface pointer.
struct FNativeConfirmationManagerPair
{
    UObject* Object;
    IFortUIManagerInterface* Interface;
};

// Original 14514eda0 returns a POINTER to the pair, not the pair by value (rax is returned
// and the caller dereferences it). Its failure path returns a static pair (148168bd8).
// The body is NOT defined in this fragment: its vtable slot 0x150 identity, the +0x188
// director member and the tail-call 141e63a20 (pair accessor) are not recovered. See NOTES.md.
const FNativeConfirmationManagerPair* NativeConfirmationResolveManager14514eda0(UObject* WorldContext);

// Original manager-widget -> CommonUISubsystem -> CommonInputManager chain. Depends on the
// resolver above, so it cannot link until that boundary is provided.
UCommonInputManager* NativeConfirmationFindInputManager14514f020(UObject* WorldContext);

class UCommonActivatableWidget;
class UCommonActivatableWidgetQueue;

// Shared original global byte, and the switcher's actual active-widget lookup.
void NativeSetConfirmationGlobalIdle147c577d4(bool bIdle);
UCommonActivatableWidget* NativeGetQueueActiveWidget1407c1910(UCommonActivatableWidgetQueue* Queue);
bool NativeGetConfirmationGlobalIdle147c577d4();
