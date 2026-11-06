// Native0x142881ff0 CONFIDENCE: high
// The native wrapper stores the enum unconditionally and marks dirty only on change.
const EFortMotionYawAxis PreviousSetting = MotionYawAxis;
MotionYawAxis = InSetting;
if (PreviousSetting != InSetting)
{
    bDirty = true;
}
