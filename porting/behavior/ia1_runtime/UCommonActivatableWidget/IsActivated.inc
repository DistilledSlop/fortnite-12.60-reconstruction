// Original body address: 0x14080bae0 (inline thunk of UCommonActivatableWidget::IsActivated, thunk 0x14080bae0)
// Offset evidence: object offset 0x288 (this_delta_total 0) is the storage byte of declared bitfield bIsActive (bool_bits offset 648, mask 0x1). The body reads the whole byte and returns it as the bool result, so the raw byte is tested for non-zero.
// CONFIDENCE: HIGH
const uint8* Raw = reinterpret_cast<const uint8*>(this);
return *reinterpret_cast<const uint8*>(Raw + 0x288) != 0;
