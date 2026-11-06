// Original exec 0x14287ea90, leaf 0x141e3a2d0; conditional Boolean and dirty-byte stores.
// CONFIDENCE: HIGH
if (this->bAllowFriendSubscriptionNudges != bAllow)
{
    this->bAllowFriendSubscriptionNudges = bAllow;
    this->bDirty = true;
}
