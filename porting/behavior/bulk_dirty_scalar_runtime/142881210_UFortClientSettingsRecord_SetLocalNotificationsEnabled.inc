// Original exec 0x142881210, leaf 0x141e3bea0; conditional Boolean and dirty-byte stores.
// CONFIDENCE: HIGH
if (this->bLocalNotificationsEnabled != bEnabled)
{
    this->bLocalNotificationsEnabled = bEnabled;
    this->bDirty = true;
}
