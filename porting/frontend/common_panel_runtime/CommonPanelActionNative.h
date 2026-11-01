#pragma once
#include "CoreMinimal.h"
#include "FCommonInputActionHandlerData.h"
#include "FCommonActionCompleteSingle.h"
#include "FCommonActionProgressSingle.h"
#include "UObject/ScriptDelegates.h"

// Descriptive names for original unreflected native delegates and record.
DECLARE_MULTICAST_DELEGATE_OneParam(FCommonPanelProgressNative, float);
DECLARE_DELEGATE_OneParam(FCommonPanelCommitNative, bool&);
DECLARE_MULTICAST_DELEGATE(FCommonPanelCompleteNative);

struct FCommonPanelTrackedDelegate
{
 FScriptDelegate DynamicDelegate;
 FDelegateHandle NativeHandle;
};
struct FCommonPanelActionRecord
{
 FCommonPanelProgressNative NativeProgress;
 FCommonPanelCommitNative NativeCommit;
 FCommonPanelCompleteNative NativeComplete;
 FCommonPanelCommitNative NativeDisabledCommit;
 FCommonInputActionHandlerData HandlerData;
 TArray<FCommonPanelTrackedDelegate> NativeBindings;
};

// FScriptDelegate equality implements the original weak-pointer/function-name
// comparison, including the case where both weak objects have expired.
inline void BindCommonHeldComplete(FCommonPanelActionRecord& Record, const FCommonActionCompleteSingle& Event)
{
 const FDelegateHandle Handle = Record.NativeComplete.AddUFunction(const_cast<UObject*>(Event.GetUObject()), Event.GetFunctionName());
 FCommonPanelTrackedDelegate& Entry = Record.NativeBindings.AddDefaulted_GetRef();
 Entry.DynamicDelegate = Event;
 Entry.NativeHandle = Handle;
}
inline void BindCommonHeldProgress(FCommonPanelActionRecord& Record, const FCommonActionProgressSingle& Event)
{
 const FDelegateHandle Handle = Record.NativeProgress.AddUFunction(const_cast<UObject*>(Event.GetUObject()), Event.GetFunctionName());
 FCommonPanelTrackedDelegate& Entry = Record.NativeBindings.AddDefaulted_GetRef();
 Entry.DynamicDelegate = Event;
 Entry.NativeHandle = Handle;
}
template<class EventType, class NativeMulticast>
void RemoveFirstCommonHeldBinding(FCommonPanelActionRecord& Record, const EventType& Event, NativeMulticast& Multicast)
{
 for (const FCommonPanelTrackedDelegate& Entry : Record.NativeBindings) {
  if (Entry.DynamicDelegate == static_cast<const FScriptDelegate&>(Event)) {
   Multicast.Remove(Entry.NativeHandle);
   break;
  }
 }
}

struct FCommonPanelActionKeyFuncs : BaseKeyFuncs<TPair<FDataTableRowHandle, FCommonPanelActionRecord>, FDataTableRowHandle, false>
{
 using ElementInitType = TTypeTraits<TPair<FDataTableRowHandle, FCommonPanelActionRecord>>::ConstInitType;
 using KeyInitType = TTypeTraits<FDataTableRowHandle>::ConstInitType;
 static KeyInitType GetSetKey(ElementInitType Element) { return Element.Key; }
 static bool Matches(KeyInitType A, KeyInitType B) { return A == B; }
 static uint32 GetKeyHash(KeyInitType Key) {
  return HashCombine(HashCombine(0, GetTypeHash(Key.RowName)), PointerHash(Key.DataTable));
 }
};

// Exposes the original map lookup's sparse element id to preserve its observed
// reuse as the initial byte of the out-only committed callback.
class FCommonPanelActionMap : public TMap<FDataTableRowHandle, FCommonPanelActionRecord, FDefaultSetAllocator, FCommonPanelActionKeyFuncs>
{
public:
 FCommonPanelActionRecord* FindWithSlot(const FDataTableRowHandle& Key, int32& Slot) {
  const FSetElementId Id = Pairs.FindId(Key);
  if (!Id.IsValidId()) { Slot = INDEX_NONE; return nullptr; }
  Slot = Id.AsInteger();
  return &Pairs[Id].Value;
 }
};
