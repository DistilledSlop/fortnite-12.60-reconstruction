// Thunk 0x1427cfd10: ADD RCX,0x3b0 (thunk_delta 0x3b0), then virtual call [RAX+0x768] on the sub-object (wrapper_delta 0, body direct).
// Body 0x1418c9a40: MOVZX EAX,byte [RCX+0xd7c]; RET. Object offset = 0x3b0 + 0xd7c = 0x112c.
// 0x112c is in the Antelope own region (after WheelBlur_FR at 0x1120, before PSC_WheelDust at 0x1168); no reflected member covers it.
// Existing generated member uint8 NativeRecoveredByte_0x112c (AFortAthenaAntelopeVehicle.h line 218, MANIFEST member_field) is used.
// CONFIDENCE: MEDIUM
return NativeRecoveredByte_0x112c != 0;
