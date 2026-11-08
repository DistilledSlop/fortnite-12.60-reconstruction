// Original body: 0x141b720d0 (thunk 0x142a090f0). Offset evidence: the weak pointer is at object offset 0x1380 (this_delta_total 0x1380); no reflected member covers it (the last reflected AFortSpaghettiVehicle member starts at 0x1190 and nothing in the class chain spans 0x1380), so it is accessed raw. First check AFortOctopusVehicle::NativeIsWeakObjectValid1406a7ed0(weak, false); second check is the weak pointer IsValid (0x142ee7880).
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
TWeakObjectPtr<UObject>* TowhookWeak = const_cast<TWeakObjectPtr<UObject>*>(reinterpret_cast<const TWeakObjectPtr<UObject>*>(Raw + 0x1380));
if (!AFortOctopusVehicle::NativeIsWeakObjectValid1406a7ed0(TowhookWeak, false))
{
	return false;
}
return TowhookWeak->IsValid();
