#pragma once

class UObject;
class UFortMcpContext;
class UCommonActivatableWidgetQueue;
class UFortConfirmationWindow;
class UFortErrorWindow;

// Exact call boundaries; these declarations intentionally have no fallback bodies.
UFortMcpContext* NativeGetMcpContext1425dc880(const UObject* WorldContext);
void NativeCancelMcpPendingRequest1421cb230(UFortMcpContext* Context);
void NativeClearModalQueue1407ae7f0(UCommonActivatableWidgetQueue* Queue);
void NativeClearConfirmation144f664f0(UFortConfirmationWindow* Confirmation);
void NativeClearErrors144d74df0(UFortErrorWindow* Errors);
