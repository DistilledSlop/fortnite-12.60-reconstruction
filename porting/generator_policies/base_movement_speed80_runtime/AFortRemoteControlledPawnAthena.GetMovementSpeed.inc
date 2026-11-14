// CONFIDENCE: high
// Original 0x14243fb10. Style is unused by this override.
if (!RemoteControlPawnSet) return 100.0f;
return bCurrentlyDelayingLaunch ? RemoteControlPawnSet->LaunchSpeed.GetCurrentValue() : RemoteControlPawnSet->MaxSpeed.GetCurrentValue();
