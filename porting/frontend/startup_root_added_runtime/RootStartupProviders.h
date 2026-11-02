#pragma once
#include "UObject/SoftObjectPtr.h"

// Typed declarations of concrete immediate dependencies. No bodies are supplied
// here for unclosed providers; policy.apply must reject their absence.
class UObject;
class UUserWidget;
class ULocalPlayer;
class UFortGlobalUIContext;
class UFortUIManager;
class UFortRootViewportLayout;
class UFortUIStateWidget_Login;
class UStartUpUIStateWidget_SubgameSelect;
class UAthenaUIStateWidget_Frontend;
class UAthenaBroadcastHUDBase;

UFortGlobalUIContext* NativeGetGlobalUIContext144ade110(const UObject* WorldContext);
UFortUIManager* NativeGetManager14514e410(const UObject* WorldContext);
UFortRootViewportLayout* NativeGetRoot14514e760(const ULocalPlayer* LocalPlayer);
void NativeRemoveWidget145174fa0(UUserWidget* Widget);
UClass* NativeLoadLoginClass145141970(const TSoftClassPtr<UFortUIStateWidget_Login>& Class, bool bLoad);
UClass* NativeLoadSubgameClass145141b80(const TSoftClassPtr<UStartUpUIStateWidget_SubgameSelect>& Class, bool bLoad);
UClass* NativeLoadFrontendClass145141760(const TSoftClassPtr<UAthenaUIStateWidget_Frontend>& Class, bool bLoad);
UClass* NativeLoadReplayClass145141550(const TSoftClassPtr<UAthenaBroadcastHUDBase>& Class, bool bLoad);
