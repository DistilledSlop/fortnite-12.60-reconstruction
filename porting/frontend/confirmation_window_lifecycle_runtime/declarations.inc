class UFortConfirmationWindow;
// Project-local native names. The original multicast at +0x5c8 is not reflected.
DECLARE_MULTICAST_DELEGATE_OneParam(FNativeConfirmationWindowClosed, UFortConfirmationWindow*);

// Shared original handle allocator and load-guard Slate update must be supplied
// by their owning native components; these are not no-op fallbacks.
class UCommonLoadGuard;
void NativeAllocateDialogLatentHandle141f87900(FFortDialogExternalLatentActionHandle& Handle);
void NativeSetLoadGuardLoading1407f7e20(UCommonLoadGuard* Guard, bool bLoading);

class UFortConfirmationWindow;
class UWidget;
void NativeConfirmationFocus144f6bdb9(UFortConfirmationWindow* Window, UWidget* FocusTarget);
void NativeConfirmationScroll144e1c6e0(UWidget* ScrollWidget);
class UFortUINavigationManager;
void NativeNavigationManagerFocus144db14e0(UFortUINavigationManager* Manager, UWidget* FocusTarget);
