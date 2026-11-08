// Helper 0x140ae9a90: bool -> FString ("true"/"false").
// Evidence: RCX = hidden sret FString, DL selects the literal: wide string at 0x1459ac118 = "true" (DL != 0), 0x1459ac128 = "false"
//   (data_consts bytes decoded as UTF-16LE); the TArray Reserve/AddUninitialized + Memcpy sequence builds FString(const TCHAR*).
// CONFIDENCE: HIGH
return bFlag ? FString(TEXT("true")) : FString(TEXT("false"));
