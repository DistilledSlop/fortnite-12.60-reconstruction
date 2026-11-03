#include "UFortConfirmationWindow.h"
#include "UFortUIManagerWidget_Campaign.h"
#include "ConfirmationImportProviders.h"
#include "ConfirmationWindowLifecycleProviders.h"

void NativeConfirmationFocus144f6bdb9(UFortConfirmationWindow* Window, UWidget* FocusTarget)
{
    const FNativeConfirmationManagerPair* Pair = NativeConfirmationResolveManager14514eda0(Window);
    UFortUIManagerWidget_Campaign* Campaign = Pair ? Cast<UFortUIManagerWidget_Campaign>(Pair->Object) : nullptr;
    UFortUINavigationManager* Manager = Campaign ? Campaign->NavigationManager : nullptr;
    if (Manager)
        NativeNavigationManagerFocus144db14e0(Manager, FocusTarget);
    else if (FocusTarget)
        FocusTarget->SetFocus();
}
