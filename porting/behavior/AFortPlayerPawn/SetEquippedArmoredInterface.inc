// CONFIDENCE: high; exec 0x142a7ff50 normalizes a null object before the two-word assignment.
ArmoredInterface = InInterface;
if (!InInterface.GetObject())
{
    ArmoredInterface.SetInterface(nullptr);
}
