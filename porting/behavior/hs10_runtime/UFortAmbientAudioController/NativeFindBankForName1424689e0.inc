// Helper 0x1424689e0 (9 callers; e.g. 0x142470450 passes controller in RCX and an FName in RDX).
// Asm: RSI=[RCX+0xb0]; 0x140696410(this+0xe0,&Idx,Key) is TSet FindId over 0x18-byte pairs (8-byte FName key compare, +8 value); Idx==-1 returns RSI else the value.
// Offset evidence: lookup_offset UFortAmbientAudioController 0xb0 = DefaultAudioBank (UFortPOIAmbientAudioBank*), 0xe0 = BankMap (TMap<FName, UFortPOIAmbientAudioBank*>).
// CONFIDENCE: HIGH
UFortPOIAmbientAudioBank* const* Found = Controller->BankMap.Find(BankName);
if (Found == nullptr)
{
	return Controller->DefaultAudioBank;
}
return *Found;
