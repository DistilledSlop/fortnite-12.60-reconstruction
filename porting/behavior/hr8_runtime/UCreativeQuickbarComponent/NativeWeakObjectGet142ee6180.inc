// Helper 0x142ee6180: TWeakObjectPtr resolve. WeakPtr layout: ObjectIndex int32 at +0x0, ObjectSerialNumber int32 at +0x4.
// Returns the item Object when serial is nonzero, index in [0, NumElements), item non-null and item serial matches.
// CONFIDENCE: HIGH - every instruction of the 54-insn body is accounted for (serial zero/range/chunk/serial compare, item->Object load).
const uint8* WeakBytes = reinterpret_cast<const uint8*>(WeakPtr);
const int32 ObjectIndex = *reinterpret_cast<const int32*>(WeakBytes);
const int32 SerialNumber = *reinterpret_cast<const int32*>(WeakBytes + 0x4);
if (SerialNumber == 0)
{
    return nullptr;
}
if (ObjectIndex < 0 || ObjectIndex >= GUObjectArray.GetObjectArrayNum())
{
    return nullptr;
}
FUObjectItem* Item = GUObjectArray.IndexToObject(ObjectIndex);
if (Item == nullptr || Item->SerialNumber != SerialNumber)
{
    return nullptr;
}
return Item->Object;
