// Body 0x14238e840 (thunk 0x142a48080). MyHUD at +0x2a8 is an APlayerController member (engine ancestor, not generated): raw access at object offset.
// Class gate 0x142b26ed0 = AFortUIZone::StaticClass(); inline IsA chain check ([obj+0x10] class, depth +0x38, chain +0x30). Helper 0x142607f40 = AFortPlayerController::NativeCanToggleFullscreenMap142607f40, called with the HUD object.
// CONFIDENCE: HIGH
static_assert(sizeof(AFortUIZone) > 0, "AFortUIZone include for IsA check");
UObject* HUD = *reinterpret_cast<UObject* const*>(reinterpret_cast<const uint8*>(this) + 0x2a8);
if (!HUD || !HUD->IsA(AFortUIZone::StaticClass()))
{
    return false;
}
return AFortPlayerController::NativeCanToggleFullscreenMap142607f40(HUD);
