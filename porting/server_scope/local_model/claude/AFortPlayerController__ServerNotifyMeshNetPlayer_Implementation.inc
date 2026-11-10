// CONFIDENCE: LOW. FUN_143f76130 and FUN_14107ab70 are read; FUN_1406a9dc0 (queue add) is not read.
// Queue the (controller, tag) pair on the registered owner object if its byte at +0xb0 is clear.
if (AActor* Owner = GetOwnerForMeshNotification(this))   // FUN_143f76130: returns GetWorld()->+0x188
{
    if (auto* Target = LookupRegisteredTarget(Owner))      // FUN_1410766f0
    {
        if (!Target->bNotifyFlag)                           // FUN_14107ab70: byte at +0xb0
        {
            Target->PendingMeshNotifications.Add({this, Tag});   // FUN_1406a9dc0(Target+0x70, pair)
        }
    }
}
