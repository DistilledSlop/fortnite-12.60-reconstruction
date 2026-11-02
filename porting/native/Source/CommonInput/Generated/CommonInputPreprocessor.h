#pragma once
#include "CommonInputPreprocessorCore.h"
#include "UCommonInputSubsystem.h"
#include "Engine/LocalPlayer.h"
#include "Framework/Application/IInputProcessor.h"
#include "Framework/Application/SlateApplication.h"
#include "Input/Events.h"
#include "InputCoreTypes.h"

// Local name: the original input preprocessor has no recovered C++ identifier.
class FCommonInputPreprocessor final : public IInputProcessor
{
public:
 explicit FCommonInputPreprocessor(UCommonInputSubsystem& InOwner) : Owner(InOwner) {}
 virtual void Tick(const float, FSlateApplication&, TSharedRef<ICursor>) override {}
 virtual bool HandleKeyDownEvent(FSlateApplication& App, const FKeyEvent& Event) override {
  TAdapter<FKeyEvent> Adapter{*this,App}; return CommonInputPreprocessorCore::KeyDown(Adapter, Event);
 }
 virtual bool HandleAnalogInputEvent(FSlateApplication& App, const FAnalogInputEvent& Event) override {
  TAdapter<FAnalogInputEvent> Adapter{*this,App}; return CommonInputPreprocessorCore::Analog(Adapter, Event);
 }
 virtual bool HandleMouseMoveEvent(FSlateApplication& App, const FPointerEvent& Event) override {
  TAdapter<FPointerEvent> Adapter{*this,App}; return CommonInputPreprocessorCore::PointerMove(Adapter, Event);
 }
 virtual bool HandleMouseButtonDownEvent(FSlateApplication& App, const FPointerEvent& Event) override {
  TAdapter<FPointerEvent> Adapter{*this,App}; return CommonInputPreprocessorCore::PointerButton(Adapter, Event);
 }
 virtual bool HandleMouseButtonDoubleClickEvent(FSlateApplication& App, const FPointerEvent& Event) override {
  TAdapter<FPointerEvent> Adapter{*this,App}; return CommonInputPreprocessorCore::PointerButton(Adapter, Event);
 }
 void IgnoreNextMove() { bIgnoreNextMove = true; }
 void SetBlocked(ECommonInputType Type, FName Reason, bool bBlocked) {
  const uint8 Index = static_cast<uint8>(Type);
  TMap<FName, bool> Copy = BlockReasons[Index];
  Copy.Add(Reason, bBlocked);
  bool Result = false;
  for (const auto& Entry : Copy) Result |= Entry.Value;
  bBlockedTypes[Index] = Result;
 }
private:
 template<class EventType> struct TAdapter {
  FCommonInputPreprocessor& P; FSlateApplication& App;
  bool IsActive() const { return App.IsActive(); }
  bool MatchesUser(const EventType& Event) const { return static_cast<uint32>(P.Owner.GetLocalPlayer<ULocalPlayer>()->GetControllerId()) == Event.GetUserIndex(); }
  bool IsSimulatedClick() const { return P.Owner.bIsGamepadSimulatedClick; }
  bool IsGamepadKey(const EventType& Event) const { return Event.GetKey().IsGamepadKey(); }
  bool IsAndroidSystemKey(const EventType& Event) const {
   const FKey Key = Event.GetKey(); return Key == EKeys::Android_Back || Key == EKeys::Android_Menu || Key == EKeys::Android_Volume_Down || Key == EKeys::Android_Volume_Up;
  }
  bool IsTouchEvent(const EventType& Event) const { return Event.IsTouchEvent(); }
  bool IsBlocked(CommonInputPreprocessorCore::Type Type) const { return P.bBlockedTypes[static_cast<uint8>(Type)]; }
  void SetInputType(CommonInputPreprocessorCore::Type Type) { P.Owner.SetCurrentInputType(static_cast<ECommonInputType>(Type)); }
  bool IgnoreNextMouseMove() const { return P.bIgnoreNextMove; }
  void ClearIgnoreNextMouseMove() { P.bIgnoreNextMove = false; }
  float AbsDeltaX(const EventType& Event) const { return FMath::Abs(Event.GetCursorDelta().X); }
  float AbsDeltaY(const EventType& Event) const { return FMath::Abs(Event.GetCursorDelta().Y); }
 };
 UCommonInputSubsystem& Owner;
 bool bIgnoreNextMove = false;
 bool bBlockedTypes[3] = {};
 TMap<FName, bool> BlockReasons[3];
};
