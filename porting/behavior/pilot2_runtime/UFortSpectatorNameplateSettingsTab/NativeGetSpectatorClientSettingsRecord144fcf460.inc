// Helper 0x144fcf460 (callers: UFortSpectatorCameraSettingsTab setters, so RCX is any UFortReplayViewSettingsTabBase widget).
// vtbl[0x288] on UFortReplayViewSettingsTabBase / UCommonActivatablePanel / UCommonActivatableWidget targets 0x143d5e160 (virtual_slots):
// `LEA RBX,[RCX+0x218]; IsValid(0x142ee7880) ? Get(0x142ee5dd0) : null`, i.e. the widget's owning-player weak pointer (unreflected,
// lookup_offset reports nothing at 0x218). It is read directly here; an override on the concrete tab class cannot be excluded because
// the concrete class has no recorded vtable.
// 0x142a69680 = AFortPlayerControllerSpectating::StaticClass(); the inline chain check is Cast<>. The tail JMP 0x14237e580 is the
// ported static helper AFortPlayerControllerGameplay::NativeGetClientSettingsRecord14237e580.
const TWeakObjectPtr<UObject>& OwnerWeakRef = *reinterpret_cast<const TWeakObjectPtr<UObject>*>(reinterpret_cast<const uint8*>(SettingsTab) + 0x218);
UObject* OwnerObject = OwnerWeakRef.IsValid() ? OwnerWeakRef.Get() : nullptr;
AFortPlayerControllerSpectating* SpectatingController = Cast<AFortPlayerControllerSpectating>(OwnerObject);
if (SpectatingController == nullptr)
{
	return nullptr;
}
return AFortPlayerControllerGameplay::NativeGetClientSettingsRecord14237e580(SpectatingController);
// CONFIDENCE: MEDIUM
