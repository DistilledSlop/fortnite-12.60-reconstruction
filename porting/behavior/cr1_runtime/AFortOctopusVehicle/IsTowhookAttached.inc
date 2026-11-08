// Original body address: 0x141b720d0
// Offset evidence: body RCX = object+0x1380 (receiver_delta 0x1380), a TWeakObjectPtr with no reflected member at 0x1380 (own region starts at 0xf38). Weak helper 0x1406a7ed0 (NativeIsWeakObjectValid1406a7ed0, bEvenIfPendingKill=false) must pass, then the weak IsValid check 0x142ee7880 decides the result.
// CONFIDENCE: HIGH
const uint8* RawThis = reinterpret_cast<const uint8*>(this);
TWeakObjectPtr<UObject>* TowhookWeakRef = const_cast<TWeakObjectPtr<UObject>*>(reinterpret_cast<const TWeakObjectPtr<UObject>*>(RawThis + 0x1380));
if (!AFortOctopusVehicle::NativeIsWeakObjectValid1406a7ed0(TowhookWeakRef, false))
{
    return false;
}
return TowhookWeakRef->IsValid();
