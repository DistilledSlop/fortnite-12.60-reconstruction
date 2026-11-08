// Original body address: 0x141b493d0
// Offset evidence: no reflected member is accessed; the receiver is passed through (RCX unchanged). Calls the static NativeFindWaxRecordByKey141b48bf0 with the 0x58-byte stack buffer at [RSP+0x20,RSP+0x78) and RecordKey = Player; returns int32 at record+0x28 (no null check in the asm).
// CONFIDENCE: HIGH
uint8 RecordBuffer[0x58];
uint8* Record = AFortAthenaMutator_Wax::NativeFindWaxRecordByKey141b48bf0(const_cast<AFortAthenaMutator_Wax*>(this), RecordBuffer, reinterpret_cast<uint64>(Player));
return *reinterpret_cast<const int32*>(Record + 0x28);
