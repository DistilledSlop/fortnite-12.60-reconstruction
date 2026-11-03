// CONFIDENCE: high; signed reflected array/string count comparison.
// AFortPlayerPawn::ServerSendAimbotDetectionStatus_Validate, original 0x14234a9d0.
// The original compares the signed int32 at Payload + 8 (TArray::ArrayNum) with 100.
return Payload.Num() < 100;
